#pragma once
#include "Box.hpp"

#include "Rifle.hpp"

class RifleTester : public Entity
{
public:
	RifleTester(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position);
	~RifleTester();

	void handleInput(float deltaTime);
	Rifle* rifle = nullptr;
};

