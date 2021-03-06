/*
 *  mirror_obj.cpp
 *  kale
 *
 *  Created by exe on 8/18/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "mirror_obj.h"

#include <cmath>

#include "root.h"
#include "renderer.h"

#include "kale.h"

using namespace ERI;

#pragma mark TriangleMirror

TriangleMirror::TriangleMirror(float half_edge, int row, int col) :
	half_edge_(half_edge), row_(row), col_(col)
{
	UpdateVertexBuffer();
}

TriangleMirror::~TriangleMirror()
{
}

void TriangleMirror::UpdateVertexBuffer()
{
	if (render_data_.vertex_buffer == 0)
	{
		glGenBuffers(1, &render_data_.vertex_buffer);
	}
	
	Vector2 unit_uv_(1.0f, 1.0f);
	
	float top_v = 0.875f;
	float bottom_v = 0.125f;
	float center = 0.5f;
	
	int v_count = row_ * col_ * 3;
	
	vertex_2_pos_tex* v = new vertex_2_pos_tex[v_count];
	vertex_2_pos_tex* v_start = v;
	
	Vector2 step(half_edge_, sqrt(3) * half_edge_);
	Vector2 start(-step.x * (col_ - 1) * 0.5f, step.y * row_ * 0.5f);
	
	bool is_invert;
	Vector2 uv[3];
	uv[2] = Vector2(center, top_v);
	uv[0] = Vector2(center - (top_v - bottom_v) / sqrt(3.0f), bottom_v);
	uv[1] = Vector2(center + (top_v - bottom_v) / sqrt(3.0f), bottom_v);
	
	int start_index;
	Vector2 uv_now[3];
	
	for (int i = 0; i < row_; ++i)
	{
		if (i % 2) start_index = 0;
		else start_index = 1;
		
		uv_now[0] = uv[start_index % 3];
		uv_now[1] = uv[(start_index + 1) % 3];
		uv_now[2] = uv[(start_index + 2) % 3];
		
		for (int j = 0; j < col_; ++j)
		{
			is_invert = (j % 2 == 0);
			if (i % 2) is_invert = !is_invert;
			
			// position
			v_start->position[0] = start.x + step.x * j;
			v_start->position[1] = start.y - step.y * (is_invert ? (i + 1) : i);
			// texture coordinate
			v_start->tex_coord[0] = is_invert ? uv_now[2].x : uv_now[0].x;
			v_start->tex_coord[1] = is_invert ? uv_now[2].y : uv_now[0].y;
			
			++v_start;
			
			// position
			v_start->position[0] = start.x + step.x * (is_invert ? (j + 1) : (j - 1));
			v_start->position[1] = start.y - step.y * (is_invert ? i : (i + 1));
			// texture coordinate
			v_start->tex_coord[0] = uv_now[1].x;
			v_start->tex_coord[1] = uv_now[1].y;
			
			++v_start;
			
			// position
			v_start->position[0] = start.x + step.x * (is_invert ? (j - 1) : (j + 1));
			v_start->position[1] = start.y - step.y * (is_invert ? i : (i + 1));
			// texture coordinate
			v_start->tex_coord[0] = is_invert ? uv_now[0].x : uv_now[2].x;
			v_start->tex_coord[1] = is_invert ? uv_now[0].y : uv_now[2].y;
			
			++v_start;
			
			if (is_invert)
			{
				++start_index;
				if (start_index == 3) start_index = 0;
				uv_now[0] = uv[start_index % 3];
				uv_now[1] = uv[(start_index + 1) % 3];
				uv_now[2] = uv[(start_index + 2) % 3];
			}
		}
	}
	
	render_data_.vertex_type = GL_TRIANGLES;
	render_data_.vertex_format = POS_TEX_2;
	render_data_.vertex_count = v_count;
	
	glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_2_pos_tex) * v_count, v, GL_STATIC_DRAW);
	
	delete [] v;
}

#pragma mark Mirror

Mirror::Mirror(kaleApp* app)
{
	ASSERT(app);
	
	float content_scale = Root::Ins().renderer()->content_scale();
	
	mirror_cam_ = new CameraActor();
	mirror_cam_->SetPos(0, app->kBoundaryHalfSize);
	mirror_cam_->SetOrthoZoom(16 * content_scale);
	
	mirror_texture_ = new RenderToTexture(0, 0, 256 * content_scale, 256 * content_scale, mirror_cam_);
	mirror_texture_->Init();
	
	mirror_dark_corner_mask_ = new SpriteActor(app->kBoundaryHalfSize * 2 + 8, app->kBoundaryHalfSize * 2 + 8);
	mirror_dark_corner_mask_->AddToScene(LAYER_UI);
	mirror_dark_corner_mask_->SetPos(Vector3(0, app->kBoundaryHalfSize * 0.7f, 2));
	mirror_dark_corner_mask_->SetMaterial("media/mask.png", FILTER_LINEAR, FILTER_LINEAR);
	
	mirror_ = new TriangleMirror(50, 13, 13);
	mirror_->AddToScene();
	mirror_->SetMaterial(mirror_texture_->texture(), FILTER_LINEAR, FILTER_LINEAR);
	mirror_->SetOpacityType(OPACITY_OPAQUE);
	
	mirror_debug_ = new SpriteActor(256, 256);
	mirror_debug_->AddToScene(LAYER_UI2);
	mirror_debug_->SetMaterial(mirror_texture_->texture());
	mirror_debug_->SetTexAreaUV(0.0f, 1.0f, 1.0f, -1.0f);
}

Mirror::~Mirror()
{
	delete mirror_debug_;
	delete mirror_;
	delete mirror_dark_corner_mask_;
	delete mirror_texture_;
	delete mirror_cam_;
}

void Mirror::Make()
{
	mirror_dark_corner_mask_->SetVisible(true);
	mirror_->SetVisible(false);
	mirror_debug_->SetVisible(false);
	
	CameraActor* scene_cam = Root::Ins().scene_mgr()->GetLayerCam(LAYER_SCENE);
	Root::Ins().scene_mgr()->SetLayerCam(LAYER_SCENE, NULL);
	
	mirror_texture_->ProcessRender();
	
	Root::Ins().scene_mgr()->SetLayerCam(LAYER_SCENE, scene_cam);
}

void Mirror::Show()
{
	mirror_dark_corner_mask_->SetVisible(false);
	mirror_->SetVisible(true);
//	mirror_debug_->SetVisible(true);
}
