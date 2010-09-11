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

#pragma mark CollisionObj

class CollisionObj : public ERI::SceneActor
{
public:
	CollisionObj();
	virtual ~CollisionObj();
	
	void Update(float delta_time);
	void OnCollisionStart(float approach_velocity);
	
	ERI::SceneActor* glow_obj() { return glow_obj_; }
	void set_glow_obj(ERI::SceneActor* obj) { glow_obj_ = obj; }

private:
	ERI::SceneActor*	glow_obj_;
	float				glow_remain_time_;
};

#pragma mark Square

class Square : public CollisionObj
{
public:
	Square(float half_edge);
	virtual ~Square();
		
private:
	void UpdateVertexBuffer();
		
	float	half_edge_;
};


#pragma mark NonUniformSquare

class NonUniformSquare : public CollisionObj
{
public:
	NonUniformSquare(float half_x, float half_y);
	virtual ~NonUniformSquare();
	
private:
	void UpdateVertexBuffer();
	
	float	half_x_, half_y_;
};


#endif // KALE_COLLISION_OBJ_H