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
#include "platform_helper.h"
#include "audio_mgr.h"

#include "mirror_obj.h"
#include "helper.h"
#include "collision_obj.h"
#include "contact_listener.h"
#include "updater.h"

using namespace ERI;

static const int kCollisionObjNum = 40;
static const float kCameraZoomMin = 0.55f;
static const float kCameraZoomMax = 2.5f;
static const float kAutoResetTime = 30.0f;

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

//#define SHOW_DEBUG
#ifdef SHOW_DEBUG
static NumberActor* fps_number;

static void UpdateFPS(float delta_time)
{
	static int frame_count = 0;
	static double frame_count_timer = 0;
	
	++frame_count;
	frame_count_timer += delta_time;
	if (frame_count_timer >= 1.0f)
	{
		//fps_number->SetNumber(static_cast<int>(frame_count / frame_count_timer));
		printf("fps: %d\n", static_cast<int>(frame_count / frame_count_timer));
		
		frame_count = 0;
		frame_count_timer = 0;
	}
}
#endif

#pragma mark kaleApp

const int kaleApp::kBoundaryHalfSize = 7;
kaleApp* kaleApp::ins_ptr_ = NULL;

kaleApp::kaleApp() :
	accelerator_g_(Vector3(0, -1, 0)),
	logo_shower_(NULL),
	is_menu_mode_(false),
	is_auto_mode_(false),
	is_sound_on_(true),
	wait_button_remain_time_(0.0f),
	auto_choose_g_remain_time_(0.0f),
	auto_reset_remain_time_(0.0f)
{
}

kaleApp::~kaleApp()
{
	Release();
}

void kaleApp::Init()
{
	LoadOption();
	
	hikaru::AudioMgr::Ins().Init();
	hikaru::AudioMgr::Ins().SetGlobalSoundVolume(0.5f);
	hikaru::AudioMgr::Ins().LoadSound("media/ding.caf");
	
	Root::Ins().input_mgr()->set_handler(this);
	
	//
	
	ERI::Root::Ins().scene_mgr()->viewport_resize_subject().AddObserver(this);

	ERI::Root::Ins().scene_mgr()->CreateLayer(LAYER_MAX);
	ERI::Root::Ins().scene_mgr()->SetLayerSortAlpha(LAYER_UI, true);
	ERI::Root::Ins().scene_mgr()->SetLayerSortAlpha(LAYER_UI2, true);
	
	//
	
	cam_ = new CameraActor;
	cam_->SetOrthoZoom(Root::Ins().renderer()->content_scale());
	Root::Ins().scene_mgr()->set_default_cam(cam_);
	
	scene_cam_ = new CameraActor;
	scene_cam_->SetOrthoZoom(Root::Ins().renderer()->content_scale());
	
	ERI::Root::Ins().scene_mgr()->SetLayerCam(LAYER_SCENE, scene_cam_);
	
	light_ = new LightActor(LightActor::POINT);
	light_->SetPos(Vector3(0, kBoundaryHalfSize, 10));
	light_->SetAmbient(Color(0.2f, 0.2f, 0.2f));
	light_->SetDiffuse(Color(1.0f, 1.0f, 1.f));
	light_->SetSpecular(Color(1.0f, 1.0f, 1.0f));
	light_->SetAttenuation(1.0f, 0.01f, 0.002f);
	light_->AddToScene();
	
	//
	
	mirror_ = new Mirror(this);
	
	//
	
	atmosphere_mask_ = new SpriteActor(kBoundaryHalfSize * 2, kBoundaryHalfSize * 2, 0, kBoundaryHalfSize);
	atmosphere_mask_->AddToScene(LAYER_MASK);
	atmosphere_mask_->SetPos(Vector3(0, 0, 1));
	atmosphere_mask_->SetDepthWrite(false);
	atmosphere_mask_->BlendAdd();
	
	atmosphere_texture_ = Root::Ins().texture_mgr()->GetTexture("media/atmosphere3.png", true);
	atmosphere_v_blender_ = new Morpher<float>(0.2f, 0.05f);
	
	//

#ifdef SHOW_DEBUG
	fps_number = new NumberActor(10, 14, false);
	fps_number->SetMaterial("media/num.png");
	fps_number->SetTexUnit(5, 7);
	fps_number->AddToScene(LAYER_UI2);
	fps_number->SetPos(Vector3(130, 210, 10));
	fps_number->SetColor(Color(0.1f, 0.1f, 0.1f));
	fps_number->BlendAdd();
#endif
	
	//
	
	InitPhysics();
	InitBoundary();
	
	//
	
	ResetCollisionObjs(true);
	
	//
	
	screen_dark_corner_mask_ = new SpriteActor(10, 10);
	screen_dark_corner_mask_->AddToScene(LAYER_MASK);
	screen_dark_corner_mask_->SetPos(Vector3(0, 0, 3));
	screen_dark_corner_mask_->SetColor(Color(1.f, 1.f, 1.f, 0.95f));
	screen_dark_corner_mask_->SetMaterial("media/mask_dark.png", FILTER_LINEAR, FILTER_LINEAR);
	
	//
	
	logo_shower_ = new LogoShower();
	black_mask_ = new BlackMask(this);
	menu_button_ = new MenuButton();
	menu_ = new Menu(this);
	
	//
	
	black_mask_->FadeOut(2.0f);
}

void kaleApp::OnTerminate()
{
	SaveOption();
}

void kaleApp::OnResignActive()
{
	SaveOption();
}

void kaleApp::Release()
{
	delete menu_;
	delete menu_button_;
	delete black_mask_;
	
	if (logo_shower_)
	{
		delete logo_shower_;
		logo_shower_ = NULL;
	}
	
	delete screen_dark_corner_mask_;
	
	ClearCollisionObjs();
	
	for (int i = 0; i < boundary_bodys_.size(); ++i)
	{
		world_->DestroyBody(boundary_bodys_[i]);
	}
	
	delete contact_listener_;
	delete world_;

#ifdef SHOW_DEBUG
	delete fps_number;
#endif
	
	delete atmosphere_v_blender_;
	Root::Ins().texture_mgr()->ReleaseTexture(atmosphere_texture_);
	delete atmosphere_mask_;
	
	delete mirror_;
	
	delete light_;
	delete scene_cam_;
	delete cam_;
	
	hikaru::AudioMgr::KillIns();
}

void kaleApp::Update(float delta_time)
{
	if (is_auto_mode_)
	{
		UpdateAuto(delta_time);
	}

#ifdef SHOW_DEBUG
	UpdateFPS(delta_time);
#endif
	
	UpdateWorldTransform(delta_time);
	UpdateAtmosphere(delta_time);

	// make mirror
	
	for (int i = 0; i < collision_objs_.size(); ++i)
	{
		collision_objs_[i]->SetVisible(true);
		collision_objs_[i]->Update(delta_time);
	}
	atmosphere_mask_->SetVisible(true);
		
	screen_dark_corner_mask_->SetVisible(false);
#ifdef SHOW_DEBUG
	fps_number->SetVisible(false);
#endif
	if (logo_shower_) logo_shower_->Hide();
	black_mask_->Hide();
	menu_button_->Hide();
	menu_->Hide();
	
	mirror_->Make();
	
	// show mirror
	
	for (int i = 0; i < collision_objs_.size(); ++i)
	{
		collision_objs_[i]->SetVisible(false);
		if (collision_objs_[i]->glow_obj())
			collision_objs_[i]->glow_obj()->SetVisible(false);
	}
	atmosphere_mask_->SetVisible(false);
	
	screen_dark_corner_mask_->SetVisible(true);
#ifdef SHOW_DEBUG
	fps_number->SetVisible(true);
#endif
	if (logo_shower_) logo_shower_->Show();
	black_mask_->Show();
	menu_button_->Show();
	menu_->Show();
	
	mirror_->Show();
	
	//
	
	if (logo_shower_)
	{
		logo_shower_->Update(delta_time);
		if (logo_shower_->is_finished())
		{
			delete logo_shower_;
			logo_shower_ = NULL;
			
			wait_button_remain_time_ = 2.0f;
		}
	}
	
	if (wait_button_remain_time_ > 0.0f)
	{
		wait_button_remain_time_ -= delta_time;
		if (wait_button_remain_time_ <= 0.0f)
		{
			if (!is_menu_mode_) menu_button_->FadeInOut();
		}
	}
	
	black_mask_->Update(delta_time);
	menu_button_->Update(delta_time);
	menu_->Update(delta_time);
}

void kaleApp::Click(const ERI::InputEvent& event)
{
	Vector3 pos = Root::Ins().scene_mgr()->ScreenToWorldPos(event.x, event.y);
	if (menu_button_->IsHit(pos))
	{
		if (is_menu_mode_)
		{
			black_mask_->FadeOut(0.2f);
			menu_button_->FadeOut();
			menu_->FadeOut();
		}
		else
		{
			black_mask_->FadeIn(0.2f, 0.5f);
			menu_button_->FadeIn();
			menu_->FadeIn();
		}
		
		is_menu_mode_ = !is_menu_mode_;
	}
	else if (is_menu_mode_)
	{
		menu_->Click(pos);
	}
	else
	{
		black_mask_->FadeInOut(0.2f, 0.4f);
		
		if (is_auto_mode_)
		{
			auto_reset_remain_time_ = kAutoResetTime;
		}
	}
}

void kaleApp::MultiMove(const ERI::InputEvent* events, int num, bool is_start)
{
	static float distance = 0;
	
	ASSERT(num > 1);
	
	float x_diff = events[1].x - events[0].x;
	float y_diff = events[1].y - events[0].y;
	float now_distance = sqrt(x_diff * x_diff + y_diff * y_diff);
	
	if (!is_start)
	{
		float now_zoom = scene_cam_->ortho_zoom() / Root::Ins().renderer()->content_scale();
		now_zoom += (now_distance - distance) * 0.005f;
		if (now_zoom < kCameraZoomMin) now_zoom = kCameraZoomMin;
		if (now_zoom > kCameraZoomMax) now_zoom = kCameraZoomMax;
		scene_cam_->SetOrthoZoom(now_zoom * Root::Ins().renderer()->content_scale());
	}
	
	distance = now_distance;
}

void kaleApp::Accelerate(const Vector3& g)
{
	if (is_auto_mode_)
		return;
	
	world_->SetGravity(b2Vec2(g.x * 20, g.y * 20));
	
	accelerator_g_ = g;
	
	for (int i = 0; i < collision_bodys_.size(); ++i)
	{
		if (!collision_bodys_[i]->IsAwake())
			collision_bodys_[i]->SetAwake(true);
	}		
}

void kaleApp::Shake()
{
	if (is_auto_mode_)
		return;
	
	Vector3 inverse_g = accelerator_g_ * -1;
	inverse_g.z = 0;
	inverse_g.Normalize();
	
	Matrix4 rotate;
	Vector3 force;
	
	for (int i = 0; i < collision_bodys_.size(); ++i)
	{
		if (!collision_bodys_[i]->IsAwake())
			collision_bodys_[i]->SetAwake(true);
		
		Matrix4::RotateAxis(rotate, RangeRandom(-80, 80), Vector3(0, 0, 1));
		force = rotate * inverse_g;
		force *= RangeRandom(10.0f, 20.0f);
		
		collision_bodys_[i]->SetLinearVelocity(b2Vec2(force.x, force.y));
	}
}

void kaleApp::OnNotified(ERI::SceneMgr::ResizeInfo& info)
{
	float screen_width = info.width / cam_->ortho_zoom();
	float screen_height = info.height / cam_->ortho_zoom();
	
	screen_dark_corner_mask_->SetSizeOffset(screen_width, screen_height);
	black_mask_->SetSize(screen_width, screen_height);
	menu_button_->RefreshScreenSize(screen_width, screen_height);
}

void kaleApp::SetIsAutoMode(bool is_auto_mode)
{
	is_auto_mode_ = is_auto_mode;
	if (is_auto_mode)
	{
		auto_choose_g_remain_time_ = 0.0f;
		auto_reset_remain_time_ = kAutoResetTime;
	}
}

void kaleApp::NotifyBlackMaskBetweenFadeInOut()
{
	ResetCollisionObjs();
	menu_button_->FadeInOut();
}

void kaleApp::InitPhysics()
{
	b2Vec2 gravity(0, 0);
	bool do_sleep = true;
	world_ = new b2World(gravity, do_sleep);
	contact_listener_ = new KaleContactListener(this);
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

void kaleApp::ResetCollisionObjs(bool is_first_time /*= false*/)
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
			glow_obj->SetVisible(false);
			glow_obj->AddToScene(LAYER_MASK);
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
	
	bool use_atmosphere_color = is_first_time ? true : RangeRandom(0, 1);
	
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

void kaleApp::UpdateAuto(float delta_time)
{
	if (auto_choose_g_remain_time_ <= 0)
	{
		static Matrix4 rotate;
		Matrix4::RotateAxis(rotate, RangeRandom(0.0f, 360.0f), Vector3(0, 0, 1));
		accelerator_g_.z = 0.0f;
		accelerator_g_.Normalize();
		accelerator_g_.z = -0.5f;
		accelerator_g_.Normalize();
		accelerator_g_ = rotate * accelerator_g_;
		
		float power = RangeRandom(5.0f, 15.0f);
		world_->SetGravity(b2Vec2(accelerator_g_.x * power, accelerator_g_.y * power));
		
		for (int i = 0; i < collision_bodys_.size(); ++i)
		{
			if (!collision_bodys_[i]->IsAwake())
				collision_bodys_[i]->SetAwake(true);
		}		
		
		auto_choose_g_remain_time_ = RangeRandom(1.0f, 3.0f);
	}
	else
	{
		auto_choose_g_remain_time_ -= delta_time;
	}
	
	if (!is_menu_mode_ && auto_reset_remain_time_ > 0.0f)
	{
		auto_reset_remain_time_ -= delta_time;
		if (auto_reset_remain_time_ <= 0.0f)
		{
			black_mask_->FadeInOut(2.0f, 2.0f);
			auto_reset_remain_time_ = kAutoResetTime;
		}
	}
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
	
	Root::Ins().renderer()->SetBgColor(color * 0.f);
	atmosphere_mask_->SetColor(color * 1.f);
	
	//
	
	menu_button_->NotifyAtmosphereChange(color);
}

void kaleApp::LoadOption()
{
	std::string path = ERI::GetHomePath();
	path += "/Documents/option.sav";
	
	FILE* f = fopen(path.c_str(), "rb");
	if (f)
	{
		fread(&is_auto_mode_, sizeof(bool), 1, f);
		fread(&is_sound_on_, sizeof(bool), 1, f);
		fclose(f);
	}
	
	SetIsAutoMode(is_auto_mode_);
}

void kaleApp::SaveOption()
{
	std::string path = ERI::GetHomePath();
	path += "/Documents/option.sav";
	
	FILE* f = fopen(path.c_str(), "wb");
	if (f)
	{
		fwrite(&is_auto_mode_, sizeof(bool), 1, f);
		fwrite(&is_sound_on_, sizeof(bool), 1, f);
		fclose(f);
	}
}
