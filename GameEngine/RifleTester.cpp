#include "stdafx.h"
#include "RifleTester.hpp"

#include "Rifle.hpp"

RifleTester::RifleTester(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position)
: Entity(renderer, physics){
	this->rifle = new Rifle(this, renderer, physics, position, 0, 0, 0);
	parts.push_back(rifle);
}

RifleTester::~RifleTester()
{
}

void RifleTester::handleInput(const Input& input, float deltaTime) {
	rifle->update(deltaTime);
}
