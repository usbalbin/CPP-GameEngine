#include "stdafx.h"
#include "RifleTester.hpp"

#include "Rifle.hpp"

RifleTester::RifleTester(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position)
: Entity(renderer, physics){
	this->rifle = new Rifle(renderer, physics, position, 0, 0, 0);
	parts.push_back(rifle);
}

RifleTester::~RifleTester()
{
}

void RifleTester::handleInput(float deltaTime) {
	rifle->update(deltaTime);
}
