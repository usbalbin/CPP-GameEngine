#include "stdafx.h"
#include "Character.hpp"


#include "Utils.hpp"

Character::Character(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll) :
	Entity(renderer, physics)
{
	const float PI_HALF = 1.57079632679;
	
	
	lowerTorso = new Cube(renderer, physics, position, bodyHalfExtents, 70);
	lowerTorso->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	lowerTorso->physicsObject->setAngularFactor(btVector3(0, 0, 0));//Make sure the body won't fall over


	glm::vec3 upperTorsoHalfExtents = bodyHalfExtents * glm::vec3(1.1f, 1.0f, 1.5f);
	upperTorsoPos = glm::vec3(0, bodyHalfExtents.y + upperTorsoHalfExtents.y, 0);
	upperTorso = new Cube(renderer, physics, position + upperTorsoPos, upperTorsoHalfExtents, 5);
	upperTorso->physicsObject->setActivationState(DISABLE_DEACTIVATION);

	upperTorsoConnection = new btHinge2Constraint(*lowerTorso->physicsObject, *upperTorso->physicsObject, toVector3(position + glm::vec3(0, upperTorsoHalfExtents.y, 0)), btVector3(0, 1, 0), btVector3(1, 0, 0));

	int motorIndex = 2;//Suspension
	upperTorsoConnection->enableSpring(motorIndex, false);
	upperTorsoConnection->setLimit(motorIndex, 0, 0);

	motorIndex = 3;//Engine
	upperTorsoConnection->enableMotor(motorIndex, true);
	upperTorsoConnection->setTargetVelocity(motorIndex, 100);
	upperTorsoConnection->setMaxMotorForce(motorIndex, 500);
	upperTorsoConnection->setServo(motorIndex, true);
	upperTorsoConnection->setServoTarget(motorIndex, 0);

	motorIndex = 5;//Steering
	upperTorsoConnection->enableSpring(motorIndex, false);
	upperTorsoConnection->setLimit(motorIndex, 0, 0);



	
	glm::vec3 headHalfExtents(0.2f, 0.2f, 0.2f);
	headPos = glm::vec3(0.0f, upperTorsoHalfExtents.y + headHalfExtents.y, 0);
	head = new Cube(renderer, physics, position + upperTorsoPos + headPos, headHalfExtents, 7);
	head->physicsObject->setActivationState(DISABLE_DEACTIVATION);

	headConnection = new btHinge2Constraint(*upperTorso->physicsObject, *head->physicsObject, toVector3(position + upperTorsoPos/* + headPos*/), btVector3(0, 0, -1), btVector3(1, 0, 0));


	motorIndex = 2;//Suspension
	headConnection->enableSpring(motorIndex, false);
	headConnection->setLimit(motorIndex, 0, 0);


	motorIndex = 3;//Engine
	headConnection->enableSpring(motorIndex, false);
	headConnection->setLimit(motorIndex, 0, 0);

	motorIndex = 5;//Steering
	headConnection->enableSpring(motorIndex, false);
	headConnection->setLimit(motorIndex, 0, 0);













	float wheelMass = 10;
	float wheelRadius = 0.5f;
	
	wheel = new Sphere(renderer, physics, position + wheelPos, wheelRadius, wheelMass, 0, 0, PI_HALF);
	wheel->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	wheel->physicsObject->setFriction(4.0f);

	wheelConnection = new btHinge2Constraint(*lowerTorso->physicsObject, *wheel->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));

	wheelConnection->setLowerLimit(+PI_HALF * 1);
	wheelConnection->setUpperLimit(-PI_HALF * 1);

	

	motorIndex = 2;//Suspension
	wheelConnection->enableSpring(motorIndex, true);
	wheelConnection->setLimit(2, -0.9, -0.7);
	wheelConnection->setStiffness(motorIndex, 20);
	wheelConnection->setDamping(motorIndex, 2.0f);
	wheelConnection->setEquilibriumPoint(motorIndex, -1);
	

	motorIndex = 3;//Engine
	wheelConnection->enableMotor(motorIndex, true);
	wheelConnection->setTargetVelocity(motorIndex, 0);
	wheelConnection->setMaxMotorForce(motorIndex, 100);

	motorIndex = 5;//Steering
	wheelConnection->enableMotor(motorIndex, true);
	wheelConnection->setTargetVelocity(motorIndex, 100);
	wheelConnection->setMaxMotorForce(motorIndex, 100);
	wheelConnection->setServo(motorIndex, true);
	wheelConnection->setServoTarget(motorIndex, 0);







	riflePos = glm::vec3(0.05f + bodyHalfExtents.x, -0.1f, -0.5f);
	rifle = new Ak47(renderer, physics, position + headPos + riflePos, 0, 0, 0);
	btHinge2Constraint* rifleConnection = new btHinge2Constraint(*upperTorso->physicsObject, *rifle->physicsObject, toVector3(position + headPos + riflePos + glm::vec3(0, 0, 0.3f)), btVector3(0, 0, -1), btVector3(1, 0, 0));



	motorIndex = 2;//Suspension
	rifleConnection->enableSpring(motorIndex, true);
	rifleConnection->setLimit(motorIndex, -0.05f, 0.05f);
	rifleConnection->setStiffness(motorIndex, 60.0f);
	rifleConnection->setDamping(motorIndex, 50.0f);
	rifleConnection->setEquilibriumPoint(motorIndex, 1);


	motorIndex = 3;//Engine
	rifleConnection->enableSpring(motorIndex, true);
	rifleConnection->setLimit(motorIndex, -0.1f, +0.1f);
	rifleConnection->setStiffness(motorIndex, 200.0f);
	rifleConnection->setDamping(motorIndex, 200.0f);
	rifleConnection->setEquilibriumPoint(motorIndex, -0.1f);

	motorIndex = 5;//Steering
	rifleConnection->enableSpring(motorIndex, false);
	rifleConnection->setLimit(motorIndex, 0, 0);











	
	parts.push_back(head);
	parts.push_back(upperTorso);
	parts.push_back(lowerTorso);
	parts.push_back(wheel);
	parts.push_back(rifle);
	addConstraint(upperTorsoConnection, true);
	addConstraint(wheelConnection, true);
	addConstraint(headConnection, true);
	
	addConstraint(rifleConnection, true);
	
}


Character::~Character()
{
}

void Character::handleInput(const Input& input, float deltaTime) {
	rifle->handleInput(input, deltaTime);

	btHinge2Constraint& constraint = *(btHinge2Constraint*)constraints[1];
	
	const float PI_HALF = 1.57079632679;

	float forward = input.leftStick.y * 10;

	float right = input.leftStick.x * 5;
	

	int motorIndex = 3;
	
	float steering = atan2(right, forward);
	float speed = 0;
	if (forward || right)
		speed = sqrt(forward * forward + right * right);
	if (input.buttonShift)
		speed *= 2;
	if (speed > 1)
		std::cout << "" << std::endl;

	static float stanceHeight = -0.9f;
	if (input.buttonCtrl)
		stanceHeight += 1.0f * deltaTime;
	else
		stanceHeight -= 1.0f * deltaTime;
	btClamp(stanceHeight, -0.8f, -0.4f);

	if (input.buttonSpace)
		stanceHeight = -1.2f;

	constraint.setLimit(2, -0.9, stanceHeight + 0.2f);

	constraint.setTargetVelocity(motorIndex, speed);
		
	motorIndex = 5;
	constraint.setServoTarget(motorIndex, steering);

	yaw += input.rightStick.x * deltaTime;
	pitch += input.rightStick.y * deltaTime;
	parts[2]->physicsObject->getWorldTransform().setRotation(btQuaternion(yaw, 0, 0));
	
	btHinge2Constraint& neckConstraint = *(btHinge2Constraint*)constraints[0];
	neckConstraint.setServoTarget(3, pitch);
}


void Character::moveTo(glm::vec3 position, float yaw)
{
	

	head->physicsObject->proceedToTransform(btTransform(btQuaternion(yaw, 0, 0), toVector3(position + upperTorsoPos + headPos)));
	//Move body
	upperTorso->physicsObject->proceedToTransform(btTransform(btQuaternion(yaw, 0, 0), toVector3(position + upperTorsoPos)));
	//parts[0]->physicsObject->getWorldTransform().setOrigin(toVector3(position));
	//parts[0]->physicsObject->getWorldTransform().setRotation(btQuaternion(yaw, 0, 0));

	//Move wheel
	lowerTorso->physicsObject->proceedToTransform(btTransform(btQuaternion(yaw, 0, 0), toVector3(position)));
	//parts[1]->physicsObject->getWorldTransform().setOrigin(toVector3(position + wheelPos));
	//parts[1]->physicsObject->getWorldTransform().setRotation(btQuaternion(yaw, 0, 0));

	wheel->physicsObject->proceedToTransform(btTransform(btQuaternion(yaw, 0, PI_HALF), toVector3(position + wheelPos)));
	rifle->physicsObject->proceedToTransform(btTransform(btQuaternion(yaw, 0, 0), toVector3(position + wheelPos)));
	
	//constraints[0]
	//*constraints[0] = btHinge2Constraint(*parts[0]->physicsObject, *parts[1]->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));
	//btHinge2Constraint* connection = new btHinge2Constraint(*parts[0]->physicsObject, *parts[1]->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));
}
