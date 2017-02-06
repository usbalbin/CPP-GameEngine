#include "stdafx.h"
#include "Box.hpp"

#include "Cube.hpp"

Box::Box(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale, float mass) : Entity(renderer, physics)
{
	parts.push_back(
		new Cube(renderer, physics, position, scale, mass)
	);
}


Box::~Box()
{
}
