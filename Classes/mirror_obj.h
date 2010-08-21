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


#endif // KALE_MIRROR_OBJ_H