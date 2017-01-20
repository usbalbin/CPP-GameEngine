#include "stdafx.h"
#include "ApcTank.hpp"

#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Utils.hpp"


ApcTank::ApcTank(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll) :
	Vehicle(renderer, physics)
{

	glm::vec3 bodyHalfExtents(1.0f, 0.25f, 2.0f);
	float bodyMass = 750;

	Cube* box = new Cube(renderer, physics, position, bodyHalfExtents, bodyMass, yaw, pitch, roll);
	box->physicsObject->setActivationState(DISABLE_DEACTIVATION);


	const float PI_HALF = 1.57079632679;
								//radius, halfWidth
	glm::vec2 wheelHalfExtents(0.45f, 0.1f);
	float wheelMass = 10.0f;
	float rideHeight = -0.5f;
	float axleLength = -wheelHalfExtents.y - 0.1f;


	Cylinder* wheelFFL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z - wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelFFR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z - wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);

	Cylinder* wheelFL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z * 0.5f - wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelFR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z * 0.5f - wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);

	Cylinder* wheelRL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z * 0.5f + wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelRR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z * 0.5f + wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);

	
	Cylinder* wheelRRL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z + wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelRRR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z + wheelHalfExtents.x), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);

	for (auto part : { wheelFFL, wheelFFR, wheelFL, wheelFR, wheelRL, wheelRR, wheelRRL, wheelRRR }) {
		part->physicsObject->setActivationState(DISABLE_DEACTIVATION);
		part->physicsObject->setFriction(1);
	}

	btVector3 posFFL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z - wheelHalfExtents.x);
	btVector3 posFFR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z - wheelHalfExtents.x);

	btVector3 posFL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z * 0.5f - wheelHalfExtents.x);
	btVector3 posFR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z * 0.5f - wheelHalfExtents.x);

	btVector3 posRL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z * 0.5f + wheelHalfExtents.x);
	btVector3 posRR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z * 0.5f + wheelHalfExtents.x);

	btVector3 posRRL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z + wheelHalfExtents.x);
	btVector3 posRRR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z + wheelHalfExtents.x);

	btHinge2Constraint* connectionFFL = new btHinge2Constraint(*box->physicsObject, *wheelFFL->physicsObject, toVector3(position) + posFFL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionFFR = new btHinge2Constraint(*box->physicsObject, *wheelFFR->physicsObject, toVector3(position) + posFFR, btVector3(0, 1, 0), btVector3(1, 0, 0));

	btHinge2Constraint* connectionFL = new btHinge2Constraint(*box->physicsObject, *wheelFL->physicsObject, toVector3(position) + posFL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionFR = new btHinge2Constraint(*box->physicsObject, *wheelFR->physicsObject, toVector3(position) + posFR, btVector3(0, 1, 0), btVector3(1, 0, 0));

	btHinge2Constraint* connectionRL = new btHinge2Constraint(*box->physicsObject, *wheelRL->physicsObject, toVector3(position) + posRL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionRR = new btHinge2Constraint(*box->physicsObject, *wheelRR->physicsObject, toVector3(position) + posRR, btVector3(0, 1, 0), btVector3(1, 0, 0));


	btHinge2Constraint* connectionRRL = new btHinge2Constraint(*box->physicsObject, *wheelRRL->physicsObject, toVector3(position) + posRRL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionRRR = new btHinge2Constraint(*box->physicsObject, *wheelRRR->physicsObject, toVector3(position) + posRRR, btVector3(0, 1, 0), btVector3(1, 0, 0));


	for (auto& connection : { connectionFFL, connectionFFR, connectionFL, connectionFR }) {//Front wheels
		connection->setLowerLimit(-PI_HALF * 0.5f);
		connection->setUpperLimit(+PI_HALF * 0.5f);

		int	motorIndex = 5;//Steering
		connection->enableMotor(motorIndex, true);
		
		connection->setServo(motorIndex, true);
		connection->setServoTarget(motorIndex, 0);
		connection->setMaxMotorForce(motorIndex, 2);

		addConstraint(connection, true);
	}

	connectionFFL->setTargetVelocity(5, 3);
	connectionFFR->setTargetVelocity(5, 3);

	connectionFL->setTargetVelocity(5, 3 * 0.5f);
	connectionFR->setTargetVelocity(5, 3 * 0.5f);

	for (auto& connection : { connectionRRL, connectionRRR, connectionRL, connectionRR }) {//Rear wheels
		connection->setLowerLimit(-PI_HALF * 0.0f);
		connection->setUpperLimit(+PI_HALF * 0.0f);
		addConstraint(connection, true);
	}
	
	for (auto& connection : { connectionFFL, connectionFFR, connectionFL, connectionFR, connectionRRL, connectionRRR, connectionRL, connectionRR }) {//Rear wheels
		int motorIndex = 2;//Suspension
		connection->enableSpring(motorIndex, true);
		connection->setLimit(motorIndex, -0.3, 1);
		connection->setStiffness(motorIndex, 2000);
		connection->setDamping(motorIndex, 2000.0f);
		connection->setEquilibriumPoint(motorIndex, -1);

		motorIndex = 3;//Engine
		connection->enableMotor(motorIndex, true);
		connection->setTargetVelocity(motorIndex, 0);
		connection->setMaxMotorForce(motorIndex, 10);
	}



	parts.push_back(box);
	parts.push_back(wheelFFL);
	parts.push_back(wheelFFR);

	parts.push_back(wheelFL);
	parts.push_back(wheelFR);
	parts.push_back(wheelRL);
	parts.push_back(wheelRR);

	parts.push_back(wheelRRL);
	parts.push_back(wheelRRR);



}

void ApcTank::handleInput(float deltaTime) {
	const float PI_HALF = 1.57079632679;

	float targetVelocity = 0;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
		targetVelocity += 20;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
		targetVelocity -= 20;

	float steering = 0;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
		steering += PI_HALF * 0.5;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
		steering -= PI_HALF * 0.5;

	int motorIndex = 3;
	for (auto& constraint : constraints) {
		((btHinge2Constraint*)constraint)->setTargetVelocity(motorIndex, -targetVelocity);
		((btHinge2Constraint*)constraint)->setMaxMotorForce(motorIndex, targetVelocity ? 10 : 0);
	}

	motorIndex = 5;
	for (auto& constraint : { constraints[0], constraints[1] }) {
		((btHinge2Constraint*)constraint)->setServoTarget(motorIndex, -steering);
	}
	
	for (auto& constraint : { constraints[2], constraints[3] }) {
		((btHinge2Constraint*)constraint)->setServoTarget(motorIndex, -steering * 0.5f);
	}
}


ApcTank::~ApcTank()
{
}
