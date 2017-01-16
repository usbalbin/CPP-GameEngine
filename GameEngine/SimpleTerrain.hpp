#pragma once
#include "Entity.hpp"
class SimpleTerrain :
	public Entity
{
public:
	SimpleTerrain(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale);
	~SimpleTerrain();
};

