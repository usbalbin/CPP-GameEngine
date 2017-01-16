#pragma once
#include "Entity.hpp"
class Box :
	public Entity
{
public:
	Box(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale, float mass);
	~Box();
};

