#pragma once
#include "Entity.hpp"
#include "Rifle.hpp"

#include "BulletDynamics\Character\btKinematicCharacterController.h"

class Character :
	public Entity
{
public:
	Character(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Character();
	void handleInput(float deltaTime);
	void mouseInput(float deltaTime);

	void moveTo(glm::vec3 position, float yaw);
private:
	Rifle* rifle;
	float rideHeight = 0.2f;
	glm::vec3 bodyHalfExtents = glm::vec3(0.2f, 1.25f, 0.1f);
	glm::vec3 wheelPos = glm::vec3(0, -bodyHalfExtents.y - rideHeight, 0);
	float pitch = 0;
	float yaw = 0;
};

