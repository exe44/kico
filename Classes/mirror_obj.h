/*
 *  mirror_obj.h
 *  kale
 *
 *  Created by exe on 8/18/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_MIRROR_OBJ_H
#define KALE_MIRROR_OBJ_H

#include "scene_actor.h"

#pragma mark TriangleMirror

class TriangleMirror : public ERI::SceneActor
{
public:
	TriangleMirror(float half_edge, int row, int col);
	virtual ~TriangleMirror();
	
private:
	void UpdateVertexBuffer();
	
	float	half_edge_;
	int		row_, col_;
};

#pragma mark Mirror

class kaleApp;

class Mirror
{
public:
	Mirror(kaleApp* app);
	~Mirror();
	
	void Make();
	void Show();
	
private:
	kaleApp*				app_ref_;
	
	ERI::CameraActor*		mirror_cam_;
	ERI::RenderToTexture*	mirror_texture_;
	ERI::SpriteActor*		mirror_dark_corner_mask_;
	
	TriangleMirror*			mirror_;
	
	ERI::SpriteActor*		mirror_debug_;
};


#endif // KALE_MIRROR_OBJ_H