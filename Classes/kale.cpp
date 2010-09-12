/*
 *  kale.cpp
 *  kale
 *
 *  Created by exe on 8/16/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "kale.h"

#include "root.h"
#include "renderer.h"
#include "scene_mgr.h"
#include "scene_actor.h"
#include "audio_manager.h"

#include "mirror_obj.h"
#include "helper.h"
#include "collision_obj.h"
#include "contact_listener.h"

using namespace ERI;

static const int kCollisionObjNum = 40;
static const int kBoundaryHalfSize = 7;
static const float kCameraZoomMin = 0.82f;
static const float kCameraZoomMax = 2.5f;

enum ObjType
{
	OBJ_TRIANGLE = 0,
	OBJ_SQUARE,
	OBJ_CONVEX_5,
	OBJ_CONVEX_6,
	OBJ_CIRCLE,
	OBJ_END
};

#pragma mark Testing Usage

static SpriteActor* mirror_texture_sprite;
static TxtActor* fps_number;

static void UpdateFPS(float delta_time)
{
	static int frame_count = 0;
	static double frame_count_timer = 0;
	
	++frame_count;
	frame_count_timer += delta_time;
	if (frame_count_timer >= 0.25f)
	{
		static char number[8];
		sprintf(number, "%d", static_cast<int>(frame_count / frame_count_timer));
		fps_number->SetTxt(number);
		
		frame_count = 0;
		frame_count_timer = 0;
	}
}

#pragma mark kaleApp

kaleApp* kaleApp::ins_ptr_ = NULL;

kaleApp::kaleApp() :
	accelerator_g_(Vector3(0, -1, 0)),
	mask_layer_(-1),
	ui_layer_(-1)
{
}

kaleApp::~kaleApp()
{
}

void kaleApp::Init()
{
	Hoimi::AudioManager::Instance().Initial();
	Hoimi::AudioManager::Instance().LoadSound("ding");
	
	Root::Ins().input_mgr()->set_handler(this);
	
	//
	
	mask_layer_ = ERI::Root::Ins().scene_mgr()->AddLayer();
	ui_layer_ = ERI::Root::Ins().scene_mgr()->AddLayer();
	
	//
	
	cam_ = new CameraActor;
	cam_->AddToScene();
	Root::Ins().scene_mgr()->SetCurrentCam(cam_);
	
	light_ = new LightActor(LightActor::POINT);
	light_->SetPos(Vector3(0, kBoundaryHalfSize, 10));
	light_->SetAmbient(Color(0.2f, 0.2f, 0.2f));
	light_->SetDiffuse(Color(1.0f, 1.0f, 0.75f));
	light_->SetSpecular(Color(1.0f, 1.0f, 1.0f));
	light_->SetAttenuation(1.0f, 0.01f, 0.002f);
	light_->AddToScene();
	
	//
	
	mirror_cam_ = new CameraActor();
	mirror_cam_->SetPos(0, kBoundaryHalfSize);
	mirror_cam_->SetOrthoZoom(16);
	
	mirror_texture_ = new RenderToTexture(256, 256, mirror_cam_);
	mirror_texture_->Init();
	
	mirror_ = new TriangleMirror(50, 7, 9);
	mirror_->AddToScene();
	mirror_->SetMaterial(mirror_texture_->texture(), FILTER_LINEAR, FILTER_LINEAR);
	
	//
	
	mirror_dark_corner_mask_ = new SpriteActor(kBoundaryHalfSize * 2 + 8, kBoundaryHalfSize * 2 + 8);
	mirror_dark_corner_mask_->AddToScene(ui_layer_);
	mirror_dark_corner_mask_->SetPos(Vector3(0, kBoundaryHalfSize * 0.7f, 2));
	mirror_dark_corner_mask_->SetMaterial("media/mask.png", FILTER_LINEAR, FILTER_LINEAR);
	
	screen_dark_corner_mask_ = new SpriteActor(320, 480);
	screen_dark_corner_mask_->AddToScene(mask_layer_);
	screen_dark_corner_mask_->SetPos(Vector3(0, 0, 3));
	screen_dark_corner_mask_->SetMaterial("media/mask_dark.png", FILTER_LINEAR, FILTER_LINEAR);
	cam_->AddChild(screen_dark_corner_mask_);
	
	atmosphere_mask_ = new SpriteActor(kBoundaryHalfSize * 2, kBoundaryHalfSize * 2, 0, kBoundaryHalfSize);
	atmosphere_mask_->AddToScene(mask_layer_);
	atmosphere_mask_->SetPos(Vector3(0, 0, 1));
	atmosphere_mask_->SetDepthWrite(false);
	atmosphere_mask_->BlendAdd();
	
	//
	
	atmosphere_texture_ = Root::Ins().texture_mgr()->GetTexture("media/atmosphere3.png", true);
	atmosphere_v_blender_ = new Morpher<float>(0.2f, 0.05f);
	
	//
	
	mirror_texture_sprite = new SpriteActor(256, 256);
	mirror_texture_sprite->AddToScene();
	mirror_texture_sprite->SetMaterial(mirror_texture_->texture());
	mirror_texture_sprite->SetTexAreaUV(0.0f, 1.0f, 1.0f, -1.0f);
	
	//
	
	fps_number = new TxtActor("0", "georgia_bold", 24, true);
	fps_number->SetTextureFilter(ERI::FILTER_LINEAR, ERI::FILTER_LINEAR);
	fps_number->AddToScene(ui_layer_);
	fps_number->SetPos(120, -200);
	fps_number->SetColor(Color(1, 1, 1));
	fps_number->SetDepthTest(false);
	cam_->AddChild(fps_number);
	
	//
	
	InitPhysics();
	InitBoundary();
	
	//
	
	ResetCollisionObjs();
}

void kaleApp::OnTerminate()
{
}

void kaleApp::Release()
{
	ClearCollisionObjs();
	
	for (int i = 0; i < boundary_bodys_.size(); ++i)
	{
		world_->DestroyBody(boundary_bodys_[i]);
	}
	
	delete contact_listener_;
	delete world_;
	
	delete fps_number;
	delete mirror_texture_sprite;
	
	delete atmosphere_v_blender_;
	Root::Ins().texture_mgr()->ReleaseTexture(atmosphere_texture_);
	
	delete atmosphere_mask_;
	delete screen_dark_corner_mask_;
	delete mirror_dark_corner_mask_;
	
	delete mirror_;
	delete mirror_texture_;
	delete mirror_cam_;
	
	delete light_;
	delete cam_;
}

void kaleApp::Update(float delta_time)
{
	UpdateFPS(delta_time);
	UpdateWorldTransform(delta_time);
	UpdateAtmosphere(delta_time);

	// create mirror
	
	for (int i = 0; i < collision_objs_.size(); ++i)
	{
		collision_objs_[i]->set_visible(true);
		collision_objs_[i]->Update(delta_time);
	}
	atmosphere_mask_->set_visible(true);
	mirror_dark_corner_mask_->set_visible(true);
	mirror_->set_visible(false);
	mirror_texture_sprite->set_visible(false);
	screen_dark_corner_mask_->set_visible(false);
	fps_number->set_visible(false);
	
	mirror_texture_->ProcessRender();
	
	// show mirror
	
	for (int i = 0; i < collision_objs_.size(); ++i)
	{
		collision_objs_[i]->set_visible(false);
		if (collision_objs_[i]->glow_obj()) collision_objs_[i]->glow_obj()->set_visible(false);
	}
	atmosphere_mask_->set_visible(false);
	mirror_dark_corner_mask_->set_visible(false);
	mirror_->set_visible(true);
	//mirror_texture_sprite->set_visible(true);
	screen_dark_corner_mask_->set_visible(true);
	fps_number->set_visible(true);
}

void kaleApp::Click(int screen_x, int screen_y)
{
	ResetCollisionObjs();
}

void kaleApp::MultiMove(const ERI::Vector2* moves, int num, bool is_start)
{
	static float distance = 0;
	
	ASSERT(num > 1);
	
	float x_diff = moves[1].x - moves[0].x;
	float y_diff = moves[1].y - moves[0].y;
	float now_distance = sqrt(x_diff * x_diff + y_diff * y_diff);
	
	if (!is_start)
	{
		float now_zoom = cam_->ortho_zoom();
		now_zoom += (now_distance - distance) * 0.005f;
		if (now_zoom < kCameraZoomMin) now_zoom = kCameraZoomMin;
		if (now_zoom > kCameraZoomMax) now_zoom = kCameraZoomMax;
		cam_->SetOrthoZoom(now_zoom);
	}
	
	distance = now_distance;
}

void kaleApp::Accelerate(float g_x, float g_y, float g_z)
{
	world_->SetGravity(b2Vec2(g_x * 20, g_y * 20));
	
	accelerator_g_.x = g_x;
	accelerator_g_.y = g_y;
	accelerator_g_.z = g_z;
	
	for (int i = 0; i < collision_bodys_.size(); ++i)
	{
		if (!collision_bodys_[i]->IsAwake())
		{
			collision_bodys_[i]->SetAwake(true);
		}
	}
}

void kaleApp::InitPhysics()
{
	b2Vec2 gravity(0, 0);
	bool do_sleep = true;
	world_ = new b2World(gravity, do_sleep);
	contact_listener_ = new KaleContactListener;
	world_->SetContactListener(contact_listener_);
}

void kaleApp::InitBoundary()
{
	{ // buttom
		b2BodyDef wall_body_def;
		wall_body_def.position.Set(0, -1);
		b2Body* body = world_->CreateBody(&wall_body_def);
		b2PolygonShape wall_box;
		wall_box.SetAsBox(kBoundaryHalfSize, 1);
		body->CreateFixture(&wall_box, 0);
		boundary_bodys_.push_back(body);
	}
	
	{ // top
		b2BodyDef wall_body_def;
		wall_body_def.position.Set(0, 15);
		b2Body* body = world_->CreateBody(&wall_body_def);
		b2PolygonShape wall_box;
		wall_box.SetAsBox(kBoundaryHalfSize, 1);
		body->CreateFixture(&wall_box, 0);
		boundary_bodys_.push_back(body);
	}
	
	{ // right
		b2BodyDef wall_body_def;
		wall_body_def.position.Set(kBoundaryHalfSize + 1, kBoundaryHalfSize);
		b2Body* body = world_->CreateBody(&wall_body_def);
		b2PolygonShape wall_box;
		wall_box.SetAsBox(1, kBoundaryHalfSize);
		body->CreateFixture(&wall_box, 0);
		boundary_bodys_.push_back(body);
	}
	
	{ // left
		b2BodyDef wall_body_def;
		wall_body_def.position.Set(-kBoundaryHalfSize - 1, kBoundaryHalfSize);
		b2Body* body = world_->CreateBody(&wall_body_def);
		b2PolygonShape wall_box;
		wall_box.SetAsBox(1, kBoundaryHalfSize);
		body->CreateFixture(&wall_box, 0);
		boundary_bodys_.push_back(body);
	}
}

void kaleApp::ResetCollisionObjs()
{
	ClearCollisionObjs();
	
	float size_x, size_y, size;
	int shape_type = RangeRandom(0, OBJ_END);
	bool mix_shape = (shape_type == OBJ_END);
	
	b2Body* body;
	CollisionObj* obj;
	CollisionObj* glow_obj;
	
	for (int i = 0; i < kCollisionObjNum; ++i)
	{
		if (mix_shape) shape_type = RangeRandom(0, OBJ_END - 1);
		
		b2BodyDef body_def;
		body_def.type = b2_dynamicBody;
		body_def.position.Set(0.0f, 4.0f);
		body = world_->CreateBody(&body_def);
		
		// Define the dynamic body fixture.
		b2FixtureDef fixture_def;
		// Set the box density to be non-zero, so it will be dynamic.
		fixture_def.density = 1.0f;
		// Override the default friction.
		fixture_def.friction = 0.3f;
		
		b2PolygonShape poly_shape;
		b2CircleShape circle_shape;
		
		glow_obj = NULL;
		
		switch (shape_type) {
			case OBJ_TRIANGLE:
				{
					size = RangeRandom(0.75f, 1.25f);
					b2Vec2 vertices[3];
					vertices[0].Set(-size, -size * 0.5f);
					vertices[1].Set(size, -size * 0.5f);
					vertices[2].Set(0.0f, size);
					poly_shape.Set(vertices, 3);
					fixture_def.shape = &poly_shape;
					
					//
					
					obj = new Square(size - 0.175f);
					obj->SetMaterial("media/triangle3.png", FILTER_LINEAR, FILTER_LINEAR);
					
					glow_obj = new Square((size - 0.175f) * 1.5f);
					glow_obj->SetMaterial("media/triangle_glow2.png", FILTER_LINEAR, FILTER_LINEAR);
				}
				break;
				
			case OBJ_SQUARE:
				{
					size_x = RangeRandom(0.55f, 1.0f);
					size_y = RangeRandom(0.55f, 1.0f);
					poly_shape.SetAsBox(size_x, size_y);
					fixture_def.shape = &poly_shape;
					
					//
					
					obj = new NonUniformSquare(size_x - 0.05f, size_y - 0.05f);
					obj->SetMaterial("media/square2.png", FILTER_LINEAR, FILTER_LINEAR);
					
					glow_obj = new NonUniformSquare((size_x - 0.05f) * 1.5f, (size_y - 0.05f) * 1.5f);
					glow_obj->SetMaterial("media/square_glow2.png", FILTER_LINEAR, FILTER_LINEAR);
				}
				break;
				
			case OBJ_CONVEX_5:
				{
					size = RangeRandom(0.5f, 1.25f);
					b2Vec2 vertices[5];
					vertices[0].Set(0, size);
					vertices[1].Set(size * -0.95f, size * 0.31f);
					vertices[2].Set(size * -0.59f, size * -0.81f);
					vertices[3].Set(size * 0.59f, size * -0.81f);
					vertices[4].Set(size * 0.95f, size * 0.31f);
					poly_shape.Set(vertices, 5);
					fixture_def.shape = &poly_shape;
					
					//
					
					obj = new Square(size - 0.125f);
					obj->SetMaterial("media/convex5.png", FILTER_LINEAR, FILTER_LINEAR);
					
					glow_obj = new Square((size - 0.125f) * 1.5f);
					glow_obj->SetMaterial("media/convex5_glow.png", FILTER_LINEAR, FILTER_LINEAR);
				}
				break;
				
			case OBJ_CONVEX_6:
				{
					b2Vec2 vertices[6];
					
					/*
					 size = RangeRandom(0.5f, 1.25f);
					 vertices[0].Set(0, size);
					 vertices[1].Set(size * -0.866f, size * 0.5f);
					 vertices[2].Set(size * -0.866f, size * -0.5f);
					 vertices[3].Set(size * 0.0f, size * -1.0f);
					 vertices[4].Set(size * 0.866f, size * -0.5f);
					 vertices[5].Set(size * 0.866f, size * 0.5f);
					 */
					
					size = RangeRandom(0.75f, 1.5f);
					vertices[0].Set(0, size);
					vertices[1].Set(size * -0.47f, size * 0.47f);
					vertices[2].Set(size * -0.47f, size * -0.47f);
					vertices[3].Set(size * 0.0f, size * -1.0f);
					vertices[4].Set(size * 0.47f, size * -0.47f);
					vertices[5].Set(size * 0.47f, size * 0.47f);
					
					poly_shape.Set(vertices, 6);
					fixture_def.shape = &poly_shape;
					
					//
					
					obj = new Square(size - 0.175f);
					obj->SetMaterial("media/convex62.png", FILTER_LINEAR, FILTER_LINEAR);
					
					glow_obj = new Square((size - 0.175f) * 1.5f);
					glow_obj->SetMaterial("media/convex6_glow2.png", FILTER_LINEAR, FILTER_LINEAR);
				}
				break;
				
			case OBJ_CIRCLE:
				{
					size = RangeRandom(0.5f, 1.0f);
					circle_shape.m_radius = size;
					fixture_def.shape = &circle_shape;
					
					//
					
					obj = new Square(size - 0.05f);
					obj->SetMaterial("media/circle.png", FILTER_LINEAR, FILTER_LINEAR);
					obj->set_collision_factor(100.0f);
					
					glow_obj = new Square((size - 0.05f) * 1.5f);
					glow_obj->SetMaterial("media/circle_glow2.png", FILTER_LINEAR, FILTER_LINEAR);
				}
				break;
				
			default:
				ASSERT(0);
				break;
		}

		// Add the shape to the body.
		body->CreateFixture(&fixture_def);
		
		collision_bodys_.push_back(body);
		
		//
		
		if (glow_obj)
		{
			glow_obj->BlendAdd();
			glow_obj->SetDepthTest(false);
			glow_obj->SetDepthWrite(false);
			glow_obj->set_visible(false);
			glow_obj->AddToScene(mask_layer_);
			obj->AddChild(glow_obj);
			obj->set_glow_obj(glow_obj);
		}
		
		obj->AddToScene();
		obj->set_accept_light(true);
		
		collision_objs_.push_back(obj);
		
		//
		
		body->SetUserData(obj);
	}
	
	//
	
	atmosphere_u_ = RangeRandom(0.0f, 1.0f);
	atmosphere_v_blender_->SetCurrent(0.5f);
	
	bool use_atmosphere_color = RangeRandom(0, 1);
	
	for (int i = 0; i < kCollisionObjNum; ++i)
	{
		Vector3 start_pos(RangeRandom(-5.5f, 5.5f), RangeRandom(1.5f, 12.5f), 0);
		
		//
		
		b2Vec2 pos(start_pos.x, start_pos.y);
		collision_bodys_[i]->SetTransform(pos, 0);
		collision_bodys_[i]->SetLinearVelocity(b2Vec2(0, 0));
		collision_bodys_[i]->SetAwake(true);
		
		//
		
		collision_objs_[i]->SetPos(start_pos);
		
		//
		
		if (use_atmosphere_color)
		{
			Color color;
			atmosphere_texture_->GetPixelColor(color, atmosphere_u_ * atmosphere_texture_->width, RangeRandom(0.5f, 0.9f) * atmosphere_texture_->height);
			collision_objs_[i]->SetColor(color);
		}
		else
		{
			collision_objs_[i]->SetColor(Color(RangeRandom(0.0f, 0.4f), RangeRandom(0.0f, 0.4f), RangeRandom(0.0f, 0.4f)));
		}
	}
	
	ASSERT(collision_objs_.size() == collision_bodys_.size());
}

void kaleApp::ClearCollisionObjs()
{
	ASSERT(collision_objs_.size() == collision_bodys_.size());
	
	for (int i = 0; i < collision_bodys_.size(); ++i)
	{
		world_->DestroyBody(collision_bodys_[i]);
		delete collision_objs_[i];
	}
	
	collision_bodys_.clear();
	collision_objs_.clear();
}

void kaleApp::UpdateWorldTransform(float delta_time)
{
	world_->Step(1.0f / 60.0f, 6, 2); 
    world_->ClearForces();

	b2Vec2 pos;
	float degree;
	Vector3 translate(0, 0, 0);
	Vector3 axis(0, 0, 1);
	
	for (int i = 0; i < collision_bodys_.size(); ++i)
	{
		if (collision_bodys_[i]->IsAwake())
		{
			pos = collision_bodys_[i]->GetPosition();
			degree = collision_bodys_[i]->GetAngle() / Math::TWO_PI * 360;
			translate.x = pos.x;
			translate.y = pos.y;

			collision_objs_[i]->SetPos(translate);
			collision_objs_[i]->SetRotate(degree, axis);
		}
	}
}

void kaleApp::UpdateAtmosphere(float delta_time)
{
	Vector3 plane_g(accelerator_g_.x, accelerator_g_.y, 0);
	accelerator_g_.Normalize();
	plane_g.Normalize();
	
	float cos = accelerator_g_.DotProduct(plane_g);
	if (cos < 0) cos *= -1;
	float atmosphere_v = accelerator_g_.z > 0 ? (cos * 0.4f) : ((1.0f - cos) * 0.4f + 0.4f);
	
	atmosphere_v_blender_->SetTarget(atmosphere_v);
	atmosphere_v_blender_->Update(delta_time);
	
	Color color;
	atmosphere_texture_->GetPixelColor(color, atmosphere_u_ * atmosphere_texture_->width, atmosphere_v_blender_->current_value() * atmosphere_texture_->height);
	Root::Ins().renderer()->SetBgColor(color * 0.0f);
	
	atmosphere_mask_->SetColor(color * 1.0f);
}
