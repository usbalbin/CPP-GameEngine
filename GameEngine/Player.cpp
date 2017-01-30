#include "stdafx.h"
#include "Player.hpp"

#include "Vehicle.hpp"
#include "Utils.hpp"


Player::Player(std::vector<Entity*>& allEntities, OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw, float pitch, float roll) : Character(renderer, physics, position, yaw, pitch, roll), 
	allEntities(allEntities)
{
}

Player::~Player()
{
}

void Player::draw()
{
	if (isInVehicle())
		return;
	Character::draw();
}

void Player::handleInput(const Input& input, float deltaTime)
{
	bool enterExitPressed = input.buttonE;

	if (isInVehicle()) {
		vehicle->handleInput(input, deltaTime);
		if (enterExitPressed && !lastEnterVehicleKeyPressed)
			exitVehicle();
	}
	else {
		Character::handleInput(input, deltaTime);
		if (enterExitPressed && !lastEnterVehicleKeyPressed)
			enterVehicle();
	}
	lastEnterVehicleKeyPressed = enterExitPressed;
}

glm::mat4 Player::getMatrix()
{
	if (isInVehicle())
		return vehicle->getMatrix();
	return Entity::getMatrix();
}

glm::mat4 Player::getTranslationMatrix()
{
	if (isInVehicle())
		return vehicle->getTranslationMatrix();
	return Character::getTranslationMatrix();
}


bool Player::isInVehicle()
{
	return vehicle != nullptr;
}

void Player::enterVehicle()
{
	const float distLimit = 10;
	const float distLimit2 = distLimit * distLimit;

	float closestDistance2 = FLT_MAX;
	Vehicle* closestVehicle = nullptr;

	for (auto entity : allEntities) {
		if (entity->isVehicle()) {
			float dist2 = distance2(entity);
			if (dist2 < closestDistance2) {
				closestDistance2 = dist2;
				closestVehicle = (Vehicle*)entity;
			}
		}
	}

	if (closestDistance2 < distLimit2) {
		vehicle = closestVehicle;
		for (auto& part : parts)
			physics->removeRigidBody(part->physicsObject);
		physics->removeConstraint(constraints[0]);
		physics->removeConstraint(constraints[1]);
	}
	
}

void Player::exitVehicle()
{
	glm::vec3 playerPos = glm::vec3(glm::vec4(-100, 0, 0, 1));


	/*for (auto& part : parts) {
		physics->addRigidBody(part->physicsObject);
	}*/
	physics->addRigidBody(parts[0]->physicsObject);
	physics->addRigidBody(parts[1]->physicsObject);

	physics->addConstraint(constraints[0]);
	

	float yaw = 0;//
	glm::vec3 position = glm::vec3(glm::vec4(8, 0, 0, 1) * vehicle->getTranslationMatrix());

	moveTo(position, yaw);
	vehicle = nullptr;
}


