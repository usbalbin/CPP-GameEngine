#include "stdafx.h"
#include "TestEntity.hpp"
#include "GenericShape.hpp"


TestEntity::TestEntity(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position) : Entity(renderer, physics)
{
	parts.push_back(new GenericShape(this, renderer, physics, "content/Vehicle/HMMWV/HMMWV.obj", position, glm::vec3(1.0f/100), 100));
}


TestEntity::~TestEntity()
{
}
