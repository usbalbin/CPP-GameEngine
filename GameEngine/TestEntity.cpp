#include "stdafx.h"
#include "TestEntity.hpp"
#include "GenericShape.hpp"


TestEntity::TestEntity(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position) : Entity(renderer, physics)
{
	parts.push_back(new GenericShape(renderer, physics, "content/HMMWV.obj", position,10));
}


TestEntity::~TestEntity()
{
}
