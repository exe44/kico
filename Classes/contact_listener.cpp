/*
 *  contact_listener.cpp
 *  kale
 *
 *  Created by exe on 8/21/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "contact_listener.h"

#include "math_helper.h"

#include "audio_mgr.h"

#include "kale.h"
#include "collision_obj.h"

using namespace ERI;

KaleContactListener::KaleContactListener(kaleApp* app) : app_ref_(app)
{
}

void KaleContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);
	
	b2PointState state1[2], state2[2];
	b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
	
	if (state2[0] == b2_addState)
	{
		const b2Body* bodyA = contact->GetFixtureA()->GetBody();
		const b2Body* bodyB = contact->GetFixtureB()->GetBody();
		
		if (bodyA->GetUserData() && bodyB->GetUserData())
		{
			b2Vec2 point = worldManifold.points[0];
			b2Vec2 vA = bodyA->GetLinearVelocityFromWorldPoint(point);
			b2Vec2 vB = bodyB->GetLinearVelocityFromWorldPoint(point);
			
			Vector2 v(vB.x - vA.x, vB.y - vA.y);
			Vector2 n(worldManifold.normal.x, worldManifold.normal.y);
			float approach_velocity = v.DotProduct(n);
			
			CollisionObj* objA = static_cast<CollisionObj*>(bodyA->GetUserData());
			CollisionObj* objB = static_cast<CollisionObj*>(bodyB->GetUserData());
			approach_velocity *= (objA->collision_factor() * objB->collision_factor());
			
			if (approach_velocity > 0.33f)
			{
				if (app_ref_->is_sound_on())
				{
					hikaru::AudioMgr::Ins().PlaySound("media/ding.caf", false, (approach_velocity - 0.33f) * 1.0f, RangeRandom(0.25f, 1.25f));
				}
				
				objA->OnCollisionStart(approach_velocity);
				objB->OnCollisionStart(approach_velocity);
			}
		}
	}
}
