#include "stdafx.h"
#include "Tank.hpp"


#include "Cylinder.hpp"
#include "Utils.hpp"

Tank::Tank(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll)
	: Vehicle(renderer, physics)
{
	glm::vec3 bodyHalfExtents(1.83f, 0.4f, 3.96f);
	float bodyMass = 60000;

	Cube* body = new Cube(renderer, physics, position, bodyHalfExtents, bodyMass, yaw, pitch, roll);
	body->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	

	setupTurret(position, yaw, pitch, roll, body, bodyHalfExtents);
	parts.push_back(body);
	

	const float PI_HALF = 1.57079632679;
	//radius, halfWidth
	glm::vec2 wheelHalfExtents(0.4f, 0.05f);
	float wheelMass = 250.0f;
	float rideHeight = -0.5f;
	float axleLength = -wheelHalfExtents.y - 0.1f;


	int axelCount = 8;
	int wheelRowCount = 3;
	float wheelSpacingX = 0.02f;
	std::vector<Cylinder*> wheels;
	float stepZ = 2.0f * bodyHalfExtents.z / (axelCount - 1);
	float stepX = 2.0f * (wheelHalfExtents.y) + wheelSpacingX;
	float wheelRowOffsetZ = stepZ / wheelRowCount;

	float wheelFriction = 2;

	for (int rowX = 0; rowX < wheelRowCount; rowX++) {
		for (int rowZ = 0; rowZ < axelCount - rowX / 2; rowZ++) {

		
			glm::vec3 wheelPos = glm::vec3(
				-bodyHalfExtents.x - axleLength + rowX * stepX,
				rideHeight + 0.5f * pow(mapToRange(-bodyHalfExtents.z + stepZ * rowZ + rowX * wheelRowOffsetZ, -bodyHalfExtents.z, +bodyHalfExtents.z, -1.0f, +1.0f), 8.0f),
				-(-bodyHalfExtents.z + stepZ * rowZ + rowX * wheelRowOffsetZ)
			);
			

			auto leftWheel = new Cylinder(renderer, physics, position + wheelPos, wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
			wheels.push_back(leftWheel);
			btHinge2Constraint* connectionLeft = new btHinge2Constraint(*body->physicsObject, *leftWheel->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));
			addConstraint(connectionLeft, true);
			wheelConstraints.push_back(connectionLeft);
			parts.push_back(leftWheel);

			leftWheel->physicsObject->setActivationState(DISABLE_DEACTIVATION);
			leftWheel->physicsObject->setFriction(wheelFriction);



			wheelPos.x *= -1.0f;
			auto rightWheel = new Cylinder(renderer, physics, position + wheelPos, wheelHalfExtents, wheelMass, 0, 0, PI_HALF);
			wheels.push_back(rightWheel);
			btHinge2Constraint* connectionRight = new btHinge2Constraint(*body->physicsObject, *rightWheel->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));
			addConstraint(connectionRight, true);
			wheelConstraints.push_back(connectionRight);
			parts.push_back(rightWheel);

			rightWheel->physicsObject->setActivationState(DISABLE_DEACTIVATION);
			rightWheel->physicsObject->setFriction(wheelFriction);
		}

	}



	for (auto& constraint : wheelConstraints) {//Wheels
		btHinge2Constraint* connection = (btHinge2Constraint*)constraint;

		connection->setLowerLimit(0);
		connection->setUpperLimit(0);

		int motorIndex = 2;//Suspension
		connection->enableSpring(motorIndex, true);
		connection->setLimit(motorIndex, -0.3, 1);
		connection->setStiffness(motorIndex, 40000);
		connection->setDamping(motorIndex, 2000.0f);
		connection->setEquilibriumPoint(motorIndex, -1);

		motorIndex = 3;//Engine
		connection->enableMotor(motorIndex, true);
		connection->setTargetVelocity(motorIndex, 0);
		connection->setMaxMotorForce(motorIndex, 100);
	}

	
}

void Tank::setupTurret(glm::vec3 position, float yaw, float pitch, float roll, Cube* body, glm::vec3 bodyHalfExtents) {
	const float PI_HALF = 1.57079632679;
	
	//Turret base
	float turretBaseMass = 1000.0f;
	glm::vec3 turretBaseHalfExtents(1.3f, 0.4f, 2.0f);
	glm::vec3 turretBasePos = glm::vec3(0.0f, bodyHalfExtents.y + turretBaseHalfExtents.y + 0.02f, +0.5f);
	Cube* turretBase = new Cube(renderer, physics, position + turretBasePos, turretBaseHalfExtents, turretBaseMass, yaw, pitch, roll);
	turretBase->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	btHinge2Constraint* turretBaseConnection = new btHinge2Constraint(*turretBase->physicsObject, *body->physicsObject, toVector3(position + glm::vec3(0.0f, turretBasePos.y, 0.0f)), btVector3(1, 0, 0), btVector3(0, 1, 0));

	//Cannon
	float cannonMass = 1190.0f;
	glm::vec2 cannonHalfExtents(0.140f, 2.64f);
	glm::vec3 cannonPos = glm::vec3(0.0f, 0.0f, -turretBaseHalfExtents.z - cannonHalfExtents.y);
	float projectileMass = 20;
	float projectileRadius = 0.120f;
	cannon = new Barrel(renderer, physics, position + turretBasePos + cannonPos, cannonHalfExtents, cannonMass, projectileRadius, projectileMass, yaw, pitch + PI_HALF, roll, 0.1s, { FireMode::FULL });
	cannon->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	btHinge2Constraint* cannonConnection = new btHinge2Constraint(*turretBase->physicsObject, *cannon->physicsObject, toVector3(position + turretBasePos + glm::vec3(0.0f, 0.0f, -turretBaseHalfExtents.z)), btVector3(0, 1, 0), btVector3(1, 0, 0));
	
	parts.push_back(cannon);
	parts.push_back(turretBase);
	
	



	//Turret yaw
	addConstraint(turretBaseConnection, false);
	turretYawConstraint = turretBaseConnection;

	turretBaseConnection->setLowerLimit(0);
	turretBaseConnection->setUpperLimit(0);

	int motorIndex = 2;//Suspension
	turretBaseConnection->setLimit(motorIndex, 0, 0);

	motorIndex = 3;//Engine
	turretBaseConnection->setDamping(motorIndex, 25000.0f);
	turretBaseConnection->enableMotor(motorIndex, true);
	turretBaseConnection->setTargetVelocity(motorIndex, 2);
	turretBaseConnection->setServo(motorIndex, true);
	turretBaseConnection->setMaxMotorForce(motorIndex, 1000);

	//Cannon pitch
	addConstraint(cannonConnection, true);
	turretPitchConstraint = cannonConnection;

	cannonConnection->setLowerLimit(0);
	cannonConnection->setUpperLimit(0);

	motorIndex = 2;//Suspension
	cannonConnection->setLimit(motorIndex, 0, 0);

	motorIndex = 3;//Engine
	cannonConnection->enableMotor(motorIndex, true);
	cannonConnection->setTargetVelocity(motorIndex, 3);
	cannonConnection->setServo(motorIndex, true);
	cannonConnection->setMaxMotorForce(motorIndex, 2000);
}

void Tank::handleInput(const Input& input, float deltaTime) {
	const float PI_HALF = 1.57079632679;

	float speed = input.rightTrigger - input.leftTrigger;
	float steering = input.leftStick.x;

	btClamp(speed, -1.0f, +1.0f);
	float direction = atan2(steering, speed);
	float leftSpeed = speed + sinf(direction);
	float rightSpeed = speed + sinf(-direction);
	btClamp(leftSpeed,  -1.0f, +1.0f);
	btClamp(rightSpeed, -1.0f, +1.0f);


	int motorIndex = 3;
	for (int i = 0; i < wheelConstraints.size();) {
		float maxSpeed = 40;

		
		((btHinge2Constraint*)wheelConstraints[i])->setTargetVelocity(motorIndex, leftSpeed * maxSpeed);
		((btHinge2Constraint*)wheelConstraints[i++])->setMaxMotorForce(motorIndex, (leftSpeed || rightSpeed) ? 1000 : 100);

		((btHinge2Constraint*)wheelConstraints[i])->setTargetVelocity(motorIndex, rightSpeed * maxSpeed);
		((btHinge2Constraint*)wheelConstraints[i++])->setMaxMotorForce(motorIndex, (leftSpeed || rightSpeed) ? 1000 : 100);
	}

	handleTurretInput(input, deltaTime);
}

void Tank::handleTurretInput(const Input& input, float deltaTime) {
	cannon->updateBarrel(input, 6);//TODO: Define constants for inputs

	
	turretPitch -= input.rightStick.y * 0.1f * deltaTime;
	turretYaw -= input.rightStick.x * 0.25f * deltaTime;

	int motorIndex = 3;

	float actualPitch = turretPitchConstraint->getAngle(0);
	float diff = 0.01f * PI;
	btClamp(turretPitch, actualPitch - diff, actualPitch + diff);
	btNormalizeAngle(turretPitch);
	
	turretPitch = btNormalizeAngle(turretPitch);
	turretPitchConstraint->setServoTarget(motorIndex, turretPitch);

	turretYaw = btNormalizeAngle(turretYaw);
	turretYawConstraint->setServoTarget(motorIndex, turretYaw);

	
}

void Tank::update(float deltaTime) {
	Entity::update(deltaTime);
}

Tank::~Tank()
{
}
