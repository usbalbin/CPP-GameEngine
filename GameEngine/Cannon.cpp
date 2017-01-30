#include "stdafx.h"
#include "Cannon.hpp"

#include "Barrel.hpp"
#include "Cube.hpp"

Cannon::Cannon(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float mass) : Entity(renderer, physics)
{
	float PI_HALF = 1.57079632679;
	
	float projectileMass = 20;
	float projectileRadius = 0.120f;
	Barrel* barrel = new Barrel(renderer, physics, position, glm::vec2(0.140f, 2.64f), mass, projectileRadius, projectileMass, 0, PI_HALF * 0, 0, 1s);
	parts.push_back(barrel);



}


Cannon::~Cannon()
{
}

void Cannon::handleInput(const Input&, float deltaTime) {
	parts[0]->update(deltaTime);
}
