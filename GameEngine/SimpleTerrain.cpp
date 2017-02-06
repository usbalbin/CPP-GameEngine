#include "stdafx.h"
#include "SimpleTerrain.hpp"

#include "Cube.hpp"


SimpleTerrain::SimpleTerrain(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, glm::vec3 scale) : Entity(renderer, physics) {
	parts.push_back(
		new Cube(renderer, physics, position, scale)
	);
}

SimpleTerrain::~SimpleTerrain()
{
}
