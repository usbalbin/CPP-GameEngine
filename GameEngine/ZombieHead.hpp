#pragma once
#include "GenericShape.hpp"
class ZombieHead :
	public GenericShape
{
public:
	ZombieHead(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position);
	~ZombieHead();
	void calcHit(btManifoldPoint& cp) override;
};

