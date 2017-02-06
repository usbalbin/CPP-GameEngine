#include "stdafx.h"
#include "Car.hpp"

#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Utils.hpp"


Car::Car(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll) : 
	Vehicle(renderer, physics)
{

	glm::vec3 bodyHalfExtents(1.0f, 0.25f, 2.0f);
	float bodyMass = 700;

	Cube* box = new Cube(renderer, physics, position, bodyHalfExtents, bodyMass, yaw, pitch, roll);
	box->physicsObject->setActivationState(DISABLE_DEACTIVATION);


	const float PI_HALF = 1.57079632679;
							//radius, width, radius
	glm::vec2 wheelHalfExtents(0.5f, 0.1f);
	float wheelMass = 20.0f;
	float rideHeight = 0;
	float axleLength = wheelHalfExtents.y + 0.1f;


	Cylinder* wheelFL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelFR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelRL = new Cylinder(renderer, physics, position + glm::vec3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	Cylinder* wheelRR = new Cylinder(renderer, physics, position + glm::vec3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z), wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
	
	for (auto part : { wheelFL, wheelFR, wheelRL, wheelRR }) {
		part->physicsObject->setActivationState(DISABLE_DEACTIVATION);
		part->physicsObject->setFriction(1.2f);
	}

	btVector3 posFL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, +bodyHalfExtents.z);
	btVector3 posFR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, +bodyHalfExtents.z);
	btVector3 posRL = btVector3(-bodyHalfExtents.x - axleLength, rideHeight, -bodyHalfExtents.z);
	btVector3 posRR = btVector3(+bodyHalfExtents.x + axleLength, rideHeight, -bodyHalfExtents.z);

	btHinge2Constraint* connectionFL = new btHinge2Constraint(*box->physicsObject, *wheelFL->physicsObject, toVector3(position) + posFL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionFR = new btHinge2Constraint(*box->physicsObject, *wheelFR->physicsObject, toVector3(position) + posFR, btVector3(0, 1, 0), btVector3(1, 0, 0));
	
	btHinge2Constraint* connectionRL = new btHinge2Constraint(*box->physicsObject, *wheelRL->physicsObject, toVector3(position) + posRL, btVector3(0, 1, 0), btVector3(1, 0, 0));
	btHinge2Constraint* connectionRR = new btHinge2Constraint(*box->physicsObject, *wheelRR->physicsObject, toVector3(position) + posRR, btVector3(0, 1, 0), btVector3(1, 0, 0));
	

	for (auto& connection : { connectionFL, connectionFR }) {//Front wheels
		connection->setLowerLimit(-PI_HALF * 0.5f);
		connection->setUpperLimit(+PI_HALF * 0.5f);

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

		motorIndex = 5;//Steering
		connection->enableMotor(motorIndex, true);
		connection->setTargetVelocity(motorIndex, 3);
		connection->setServo(motorIndex, true);
		connection->setServoTarget(motorIndex, 0);
		connection->setMaxMotorForce(motorIndex, 2);

		addConstraint(connection, true);
	}


	
	for (auto& connection : { connectionRL, connectionRR }) {//Rear wheels
		connection->setLowerLimit(-PI_HALF * 0.0f);
		connection->setUpperLimit(+PI_HALF * 0.0f);
		

		int motorIndex = 2;//Suspension
		connection->enableSpring(motorIndex, true);
		connection->setLimit(motorIndex, -0.1, 1);
		connection->setStiffness(motorIndex, 2000);
		connection->setDamping(motorIndex, 2000.0f);
		connection->setEquilibriumPoint(motorIndex, -1);

		motorIndex = 3;//Engine
		connection->enableMotor(motorIndex, true);
		connection->setTargetVelocity(motorIndex, 0);
		connection->setMaxMotorForce(motorIndex, 10);

		addConstraint(connection, true);
	}



	parts.push_back(box);
	parts.push_back(wheelFL);
	parts.push_back(wheelFR);
	parts.push_back(wheelRL);
	parts.push_back(wheelRR);
	


}

void Car::handleInput(const Input& input, float deltaTime) {
	const float PI_HALF = 1.57079632679;

	float speed = input.leftStick.y - input.leftTrigger + input.rightTrigger;
	float steering = input.leftStick.x;

	btClamp(speed, -1.0f, +1.0f);

	int motorIndex = 3;
	for (auto& constraint : constraints) {
		((btHinge2Constraint*)constraint)->setTargetVelocity(motorIndex, -speed * 140);
		((btHinge2Constraint*)constraint)->setMaxMotorForce(motorIndex, std::abs(speed) * 10);
	}

	motorIndex = 5;
	steering = -PI_HALF * 0.5f * steering;
	for (auto& constraint : { constraints[0], constraints[1] }) {
		((btHinge2Constraint*)constraint)->setServoTarget(motorIndex, steering);
	}

	motorIndex = 3;
	if (input.buttonSpace) {
		for (auto& constraint : { constraints[2], constraints[3] }) {
			((btHinge2Constraint*)constraint)->setTargetVelocity(motorIndex, 0);
			((btHinge2Constraint*)constraint)->setMaxMotorForce(motorIndex, 200);
		}
	}
}


Car::~Car()
{
}
