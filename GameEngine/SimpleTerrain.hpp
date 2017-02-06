#pragma once
#include "Entity.hpp"
class SimpleTerrain :
	public Entity
{
public:
	SimpleTerrain(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale);
	~SimpleTerrain();
};

