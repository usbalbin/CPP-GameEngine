#pragma once
#include "Entity.hpp"

#include "BulletDynamics\Character\btKinematicCharacterController.h"

class Character :
	public Entity
{
public:
	Character(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Character();
	void handleInput(float deltaTime);
	void mouseInput(float deltaTime);

private:
	Entity* vehicle = nullptr;
	float pitch = 0;
};

