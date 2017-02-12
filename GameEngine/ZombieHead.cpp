#include "stdafx.h"
#include "ZombieHead.hpp"


ZombieHead::ZombieHead(Entity * parent, ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position) : GenericShape(parent, renderer, physics, "content/Character/Zombie/zombieHead.obj", position, glm::vec3(0.09), 4.0f)
{
}

ZombieHead::~ZombieHead()
{
}

void ZombieHead::calcHit(btManifoldPoint & cp) {
	parent->damage(1);
}
