#include "stdafx.h"
#include "Shape.hpp"

#include "Utils.hpp"

ClRayTracer* Shape::renderer;
btDiscreteDynamicsWorld* Shape::physics;

Shape::Shape()
{
	
}


Shape::Shape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics)
{
	this->renderer = renderer;
	this->physics = physics;
}


void Shape::initialize(Instance & graphicsObject, btRigidBody * physicsObject)
{
	this->graphicsObject = graphicsObject;
	if (!physicsObject)
		return;

	this->physicsObject = physicsObject;
	this->physicsShape = physicsObject->getCollisionShape();

	physics->addRigidBody(physicsObject);
}

Shape::~Shape()
{
	if (physicsObject) {
		auto collisionShape = physicsObject->getCollisionShape();
		disablePhysics();
		delete collisionShape;
		
	}
	else if(physicsShape){
		delete physicsShape;
	}
}

void Shape::update(float deltaTime){

}

void Shape::draw(){
	if (!graphicsObject.isInitialized())
		throw "Tried to draw non initialized object";

	glm::mat4 matrix = getMatrix();
	graphicsObject.modelMatrix = matrix;
	graphicsObject.invModelMatrix = glm::inverse(matrix);
	renderer->push_back(graphicsObject);
}

void Shape::draw(glm::mat4 matrix) {
	if (!graphicsObject.isInitialized())
		throw "Tried to draw non initialized object";

	graphicsObject.modelMatrix = matrix;
	graphicsObject.invModelMatrix = glm::inverse(matrix);
	renderer->push_back(graphicsObject);
}

void Shape::disablePhysics()
{
	auto motionState = physicsObject->getMotionState();
	auto collisionShape = physicsObject->getCollisionShape();


	physics->removeRigidBody(physicsObject);
	delete physicsObject;
	physicsObject = nullptr;

	delete motionState;

}

glm::mat4 Shape::getTranslationMatrix() {
	btTransform transform;
	transform = physicsObject->getWorldTransform();

	return toMatrix(transform);
}

glm::mat4 Shape::getInvertedTranslationMatrix() {
	return glm::inverse(getTranslationMatrix());
}

glm::mat4 Shape::getMatrix() {
	return glm::scale(glm::mat4(1), scale) * getTranslationMatrix();
}

glm::vec3 Shape::getPosition()
{
	return toVec3(physicsObject->getWorldTransform().getOrigin());
}

glm::vec3 Shape::getScale()
{
	return scale;
}

btCollisionShape* Shape::getCollisionShape()
{
	return physicsShape;
}
