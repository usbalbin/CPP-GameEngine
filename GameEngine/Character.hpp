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
	glm::mat4 cameraMatrix(float yaw, float pitch, bool firstPerson = true) override;
	
	float getYaw() { return yaw; }
	float getPitch() { return pitch; }
	virtual void damage(float damage) override;
	virtual bool justDied() override;
	virtual bool isDead() { return health <= 0; }
protected:
	float health = 1.0f;
	bool turnedIntoGarbage = false;


	float rideHeight = 0.2f;
	glm::vec3 bodyHalfExtents = glm::vec3(0.3f, 0.25f, 0.075f);
	

	Shape* head = nullptr;
	glm::vec3 headPos;
	btHinge2Constraint* headConnection = nullptr;

	Shape* upperTorso = nullptr;
	glm::vec3 upperTorsoPos;
	btHinge2Constraint* upperTorsoConnection = nullptr;

	Shape* lowerTorso = nullptr;

	Shape* wheel = nullptr;
	glm::vec3 wheelPos = glm::vec3(0, -bodyHalfExtents.y - rideHeight, 0);
	btHinge2Constraint* wheelConnection = nullptr;

	Ak47* rifle = nullptr;
	glm::vec3 riflePos;
	btHinge2Constraint* rifleConnection = nullptr;


	float pitch = 0;
	float yaw = 0;
};

