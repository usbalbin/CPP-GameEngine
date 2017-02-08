#pragma once
#include "Entity.hpp"
class TestEntity :
	public Entity
{
public:
	TestEntity(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position);
	~TestEntity();
};

