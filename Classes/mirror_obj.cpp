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

using namespace ERI;


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
	
	float top_v = 0.85f;
	float bottom_v = 0.15f;
	float center = 0.5f;
	
	int v_count = row_ * col_ * 3;
	
	vertex_2_pos_tex* v = new vertex_2_pos_tex[v_count];
	vertex_2_pos_tex* v_start = v;
	
	Vector2 step(half_edge_, sqrt(3) * half_edge_);
	Vector2 start(-step.x * (col_ - 1) * 0.5f, step.y * row_ * 0.5f);
	
	bool is_invert;
	Vector2 uv[3];
	uv[0] = Vector2(center, bottom_v);
	uv[1] = Vector2(center - (top_v - bottom_v) / sqrt(3.0f), top_v);
	uv[2] = Vector2(center + (top_v - bottom_v) / sqrt(3.0f), top_v);
	
	int start_index;
	Vector2 uv_now[3];
	
	for (int i = 0; i < row_; ++i)
	{
		if (i % 2) start_index = 1;
		else start_index = 0;
		
		uv_now[0] = uv[start_index % 3];
		uv_now[1] = uv[(start_index + 1) % 3];
		uv_now[2] = uv[(start_index + 2) % 3];
		
		for (int j = 0; j < col_; ++j)
		{
			is_invert = j % 2;
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
	
	//	render_data_.index_count = v_count;
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data_.index_buffer);
}
