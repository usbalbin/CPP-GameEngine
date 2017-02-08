#pragma once
#include "Entity.hpp"
#include "Rifle.hpp"
#include "Ak47.hpp"

#include "Cube.hpp"
#include "Sphere.hpp"

#include "BulletDynamics\Character\btKinematicCharacterController.h"

class Character :
	public Entity
{
public:
	Character(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Character();
	void handleInput(const Input& input, float deltaTime) override;
	void moveTo(glm::vec3 position, float yaw);
protected:
	float rideHeight = 0.2f;
	glm::vec3 bodyHalfExtents = glm::vec3(0.3f, 0.25f, 0.075f);
	

	Cube* head;
	glm::vec3 headPos;
	btHinge2Constraint* headConnection;

	Cube* upperTorso;
	glm::vec3 upperTorsoPos;
	btHinge2Constraint* upperTorsoConnection;

	Cube* lowerTorso;

	Sphere* wheel;
	glm::vec3 wheelPos = glm::vec3(0, -bodyHalfExtents.y - rideHeight, 0);
	btHinge2Constraint* wheelConnection;

	Ak47* rifle;
	glm::vec3 riflePos;
	btHinge2Constraint* rifleConnection;


	float pitch = 0;
	float yaw = 0;
};

