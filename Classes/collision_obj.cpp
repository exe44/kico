/*
 *  collision_obj.cpp
 *  kale
 *
 *  Created by exe on 8/18/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "collision_obj.h"

using namespace ERI;

#pragma mark CollisionObj

CollisionObj::CollisionObj() : glow_obj_(NULL), glow_remain_time_(0.0f)
{
}

CollisionObj::~CollisionObj()
{
	if (glow_obj_) delete glow_obj_;
}

void CollisionObj::Update(float delta_time)
{
	if (!glow_obj_ || glow_remain_time_ <= 0.0f)
		return;

	glow_remain_time_ -= delta_time;
	
	if (glow_remain_time_ <= 0.0f)
	{
		//glow_obj_->set_visible(false);
	}
	else
	{
		glow_obj_->SetColor(GetColor() * (glow_remain_time_ / 1.0f) * 1.0f);
		glow_obj_->set_visible(true);
	}
}

void CollisionObj::OnCollisionStart(float approach_velocity)
{
	if (!glow_obj_)
		return;
	
	glow_remain_time_ = 1.0f;
	glow_obj_->SetColor(GetColor() * 1.0f);
}

#pragma mark Square

Square::Square(float half_edge) : half_edge_(half_edge)
{
	UpdateVertexBuffer();
}

Square::~Square()
{
}

void Square::UpdateVertexBuffer()
{
	if (render_data_.vertex_buffer == 0)
	{
		glGenBuffers(1, &render_data_.vertex_buffer);
	}
	
	Vector2 unit_uv_(1.0f, 1.0f);
	
	Vector3 top_left_normal(-1, 1, 1);
	top_left_normal.Normalize();
	Vector3 top_right_normal(1, 1, 1);
	top_right_normal.Normalize();
	Vector3 bottom_left_normal(-1, -1, 1);
	bottom_left_normal.Normalize();
	Vector3 bottom_right_normal(1, -1, 1);
	bottom_right_normal.Normalize();
	
	vertex_3_pos_normal_tex v[12] =
	{
		// top left
		{ -half_edge_, half_edge_, 0, top_left_normal.x, top_left_normal.y, top_left_normal.z, 0.0f, 0.0f },
		// top right
		{ half_edge_, half_edge_, 0, top_right_normal.x, top_right_normal.y, top_right_normal.z, 1.0f, 0.0f },
		// top left
		{ -half_edge_, -half_edge_, 0, bottom_left_normal.x, bottom_left_normal.y, bottom_left_normal.z, 0.0f, 1.0f },
		// top right
		{ half_edge_, -half_edge_, 0, bottom_right_normal.x, bottom_right_normal.y, bottom_right_normal.z, 1.0f, 1.0f },
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
	
	if (render_data_.index_buffer == 0)
	{
		glGenBuffers(1, &render_data_.index_buffer);
	}
	
	//  0 -- 1
	//  |    |
	//  2 -- 3
	
	unsigned short indices[4] =
	{
		2, 3, 0, 1
	};
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data_.index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	render_data_.vertex_type = GL_TRIANGLE_STRIP;
	render_data_.vertex_format = POS_NORMAL_TEX_3;
	render_data_.vertex_count = 4;
	render_data_.index_count = 4;
}


#pragma mark NonUniformSquare

NonUniformSquare::NonUniformSquare(float half_x, float half_y) : half_x_(half_x), half_y_(half_y)
{
	UpdateVertexBuffer();
}

NonUniformSquare::~NonUniformSquare()
{
}

void NonUniformSquare::UpdateVertexBuffer()
{
	if (render_data_.vertex_buffer == 0)
	{
		glGenBuffers(1, &render_data_.vertex_buffer);
	}
	
	Vector2 unit_uv_(1.0f, 1.0f);
	
	Vector3 top_normal(0, 1, 1);
	top_normal.Normalize();
	Vector3 bottom_normal(0, -1, 1);
	bottom_normal.Normalize();
	Vector3 left_normal(-1, 0, 1);
	left_normal.Normalize();
	Vector3 right_normal(1, 0, 1);
	right_normal.Normalize();
	
	Vector3 top_left_normal(-1, 1, 1);
	top_left_normal.Normalize();
	Vector3 top_right_normal(1, 1, 1);
	top_right_normal.Normalize();
	Vector3 bottom_left_normal(-1, -1, 1);
	bottom_left_normal.Normalize();
	Vector3 bottom_right_normal(1, -1, 1);
	bottom_right_normal.Normalize();
	
	float center_u1, center_u2, center_v1, center_v2;
	if (half_x_ < half_y_)
	{
		center_u1 = center_u2 = 0.5f;
		center_v1 = half_y_ / half_x_ * 0.5f;
		center_v2 = 1 - center_v1;
	}
	else
	{
		center_v1 = center_v2 = 0.5f;
		center_u1 = half_x_ / half_y_ * 0.5f;
		center_u2 = 1 - center_u1;
	}
	
	vertex_3_pos_normal_tex v[16] =
	{
		// top left
		{ -half_x_, half_y_, 0, top_left_normal.x, top_left_normal.y, top_left_normal.z, 0.0f, 0.0f },
		// top 1
		{ 0, half_y_, 0, top_normal.x, top_normal.y, top_normal.z, center_u1, 0.0f },
		// top 2
		{ 0, half_y_, 0, top_normal.x, top_normal.y, top_normal.z, center_u2, 0.0f },
		// top right
		{ half_x_, half_y_, 0, top_right_normal.x, top_right_normal.y, top_right_normal.z, 1.0f, 0.0f },
		
		// left 1
		{ -half_x_, 0, 0, left_normal.x, left_normal.y, left_normal.z, 0.0f, center_v1 },
		// center 1-1
		{ 0, 0, 0, 0, 0, 1, center_u1, center_v1 },
		// center 1-2
		{ 0, 0, 0, 0, 0, 1, center_u2, center_v1 },
		// right 1
		{ half_x_, 0, 0, right_normal.x, right_normal.y, right_normal.z, 1.0f, center_v1 },
		
		// left 2
		{ -half_x_, 0, 0, left_normal.x, left_normal.y, left_normal.z, 0.0f, center_v2 },
		// center 2-1
		{ 0, 0, 0, 0, 0, 1, center_u1, center_v2 },
		// center 2-2
		{ 0, 0, 0, 0, 0, 1, center_u2, center_v2 },
		// right 2
		{ half_x_, 0, 0, right_normal.x, right_normal.y, right_normal.z, 1.0f, center_v2 },
		
		// bottom left
		{ -half_x_, -half_y_, 0, bottom_left_normal.x, bottom_left_normal.y, bottom_left_normal.z, 0.0f, 1.0f },
		// bottom 1
		{ 0, -half_y_, 0, bottom_normal.x, bottom_normal.y, bottom_normal.z, center_u1, 1.0f },
		// bottom 2
		{ 0, -half_y_, 0, bottom_normal.x, bottom_normal.y, bottom_normal.z, center_u2, 1.0f },
		// bottom right
		{ half_x_, -half_y_, 0, bottom_right_normal.x, bottom_right_normal.y, bottom_right_normal.z, 1.0f, 1.0f },
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
	
	if (render_data_.index_buffer == 0)
	{
		glGenBuffers(1, &render_data_.index_buffer);
	}
	
	//  0 - 1 2 - 3
	//  |    |    |
	//  4 - 5 6 - 7
	//  8 - 9 A - B
	//  |    |    |
	//  C - D E - F
	
	unsigned short indices[24] =
	{
		0, 4, 5, 5, 1, 0,
		2, 6, 7, 7, 3, 2,
		8, 12, 13, 13, 9, 8,
		10, 14, 15, 15, 11, 10
	};
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data_.index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	render_data_.vertex_type = GL_TRIANGLES;
	render_data_.vertex_format = POS_NORMAL_TEX_3;
	render_data_.vertex_count = 16;
	render_data_.index_count = 24;
}
