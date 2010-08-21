/*
 *  contact_listener.h
 *  kale
 *
 *  Created by exe on 8/21/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef KALE_CONTACT_LISTENER_H
#define KALE_CONTACT_LISTENER_H

#include <Box2D/Box2D.h>

class KaleContactListener : public b2ContactListener
{
public:
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
};

#endif // KALE_CONTACT_LISTENER_H