#include "stdafx.h"
#include "Character.hpp"

#include "Cube.hpp"
#include "Sphere.hpp"
#include "Utils.hpp"

Character::Character(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll) :
	Entity(renderer, physics)
{
	const float PI_HALF = 1.57079632679;
	glm::vec3 bodyHalfExtents(0.2f, 1.25f, 0.1f);

	Cube* body = new Cube(renderer, physics, position, bodyHalfExtents, 70);
	body->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	body->physicsObject->setAngularFactor(btVector3(0, 0, 0));//Make sure the body won't fall over


	float wheelMass = 10;
	float rideHeight = 0.2f;
	float wheelRadius = 0.5f;
	glm::vec3 wheelPos(0, -bodyHalfExtents.y - rideHeight, 0);
	Sphere* wheel = new Sphere(renderer, physics, position + wheelPos, wheelRadius, wheelMass, 0, 0, PI_HALF);
	wheel->physicsObject->setActivationState(DISABLE_DEACTIVATION);
	wheel->physicsObject->setFriction(4.0f);

	btHinge2Constraint* connection = new btHinge2Constraint(*body->physicsObject, *wheel->physicsObject, toVector3(position + wheelPos), btVector3(0, 1, 0), btVector3(1, 0, 0));




	connection->setLowerLimit(+PI_HALF * 1);
	connection->setUpperLimit(-PI_HALF * 1);

	

	int motorIndex = 2;//Suspension
	connection->enableSpring(motorIndex, true);
	connection->setLimit(2, -0.9, -0.7);
	connection->setStiffness(motorIndex, 20);
	connection->setDamping(motorIndex, 2.0f);
	connection->setEquilibriumPoint(motorIndex, -1);
	

	motorIndex = 3;//Engine
	connection->enableMotor(motorIndex, true);
	connection->setTargetVelocity(motorIndex, 0);
	connection->setMaxMotorForce(motorIndex, 100);

	motorIndex = 5;//Steering
	connection->enableMotor(motorIndex, true);
	connection->setTargetVelocity(motorIndex, 100);
	connection->setMaxMotorForce(motorIndex, 100);
	connection->setServo(motorIndex, true);
	connection->setServoTarget(motorIndex, 0);


	parts.push_back(body);
	parts.push_back(wheel);
	addConstraint(connection, true);
}


Character::~Character()
{
}

void Character::handleInput(float deltaTime) {
	btHinge2Constraint& constraint = *(btHinge2Constraint*)constraints[0];
	
	const float PI_HALF = 1.57079632679;

	float forward = 0;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_W) == GLFW_PRESS)
		forward += 20;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_S) == GLFW_PRESS)
		forward -= 20;

	float right = 0;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_D) == GLFW_PRESS)
		right += 20;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_A) == GLFW_PRESS)
		right -= 20;

	int motorIndex = 3;
	
	float steering = atan2(right, forward);
	float speed = 0;
	if (forward || right)
		speed = 5;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed *= 2;

	static float stanceHeight = -0.9f;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		stanceHeight += 1.0f * deltaTime;
	else
		stanceHeight -= 1.0f * deltaTime;
	btClamp(stanceHeight, -0.8f, -0.4f);

	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
		stanceHeight = -1.2f;

	constraint.setLimit(2, -0.9, stanceHeight + 0.2f);

	constraint.setTargetVelocity(motorIndex, speed);
		
	motorIndex = 5;
	constraint.setServoTarget(motorIndex, steering);

	float yaw = 0;

	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
		yaw += 1 * deltaTime;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
		yaw -= 1 * deltaTime;
	
	mouseInput(deltaTime);
}

void Character::mouseInput(float deltaTime) {
	static double lastPosX = 0, lastPosY = 0;
	double posX = 0, posY = 0;
	glfwGetCursorPos(renderer->getWindow(), &posX, &posY);
	float yaw = 0;

	if (glfwGetMouseButton(renderer->getWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
		yaw = (posX - lastPosX) * deltaTime;
		pitch = (posY - lastPosY) * deltaTime;
	}
	lastPosX = posX;
	lastPosY = posY;


	btTransform rotation(btQuaternion(yaw, 0, 0));
	btTransform& transform = parts[0]->physicsObject->getWorldTransform();
	transform *= rotation;
}