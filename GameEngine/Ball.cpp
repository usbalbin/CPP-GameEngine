#include "stdafx.h"
#include "Ball.hpp"

#include "Sphere.hpp"

Ball::Ball(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float scale, float mass) : Entity(renderer, physics)
{
	parts.push_back(
		new Sphere(renderer, physics, position, scale, mass)
	);
}

Ball::~Ball()
{
}
