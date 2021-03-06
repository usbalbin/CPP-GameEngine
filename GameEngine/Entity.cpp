#include "stdafx.h"
#include "Entity.hpp"
#include "Utils.hpp"


Entity::Entity(){
}

Entity::Entity(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics){
	this->physics = physics;
	this->renderer = renderer;
}

void Entity::addConstraint(btTypedConstraint* constraint, bool disableCollisionBetweenLinkedBodies) {
	physics->addConstraint(constraint, disableCollisionBetweenLinkedBodies);
	constraints.push_back(constraint);
}


Entity::~Entity() {
	if (!constraints.empty()) {
		for (auto& constraint : constraints) {
			physics->removeConstraint(constraint);
			delete constraint;
		}
	}
	if (!parts.empty()) {
		for (auto& part : parts)
			delete part;
	}
}

void Entity::handleInput(const Input& input, float deltaTime)
{
}

glm::mat4 Entity::getTranslationMatrix()
{
	return parts[0]->getTranslationMatrix();
}

glm::mat4 Entity::getMatrix()
{
	return parts[0]->getMatrix();
}

glm::vec3 Entity::getPosition()
{
	return parts[0]->getPosition();
}

void Entity::update(float deltaTime)
{
	for (auto& part : parts)
		part->update(deltaTime);
}

void Entity::draw() const{
	for (auto& part : parts)
		part->draw();
}

float Entity::distance2(Entity * other)
{
	return glm::distance2(this->getPosition(), other->getPosition());

}

glm::mat4 Entity::cameraMatrix(float yaw, float pitch, bool firstPerson)
{
	glm::vec3 relativeOffset(0, 0, 5);
	glm::vec3 rotationPivotPos(0, 0, 0);

	return cameraMatrix(yaw, pitch, relativeOffset, rotationPivotPos);
}

bool Entity::justDied()
{
	return false;
}

glm::mat4 Entity::cameraMatrix(float yaw, float pitch, glm::vec3 & relativeOffset, glm::vec3 & rotationPivotPos)
{
	return toMatrix(relativeOffset) * glm::rotate(glm::rotate(glm::mat4(1), pitch, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1, 0)) * toMatrix(rotationPivotPos) * getTranslationMatrix();
}

bool Entity::isServer()
{
	return !renderer;
}
