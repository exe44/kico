/*
 *  collision_obj.h
 *  kale
 *
 *  Created by exe on 8/18/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_COLLISION_OBJ_H
#define KALE_COLLISION_OBJ_H

#include "scene_actor.h"

#pragma mark Square

class Square : public ERI::SceneActor
{
public:
	Square(float half_edge);
	virtual ~Square();
		
private:
	void UpdateVertexBuffer();
		
	float	half_edge_;
};


#pragma mark NonUniformSquare

class NonUniformSquare : public ERI::SceneActor
{
public:
	NonUniformSquare(float half_x, float half_y);
	virtual ~NonUniformSquare();
	
private:
	void UpdateVertexBuffer();
	
	float	half_x_, half_y_;
};


#endif // KALE_COLLISION_OBJ_H