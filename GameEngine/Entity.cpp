#include "stdafx.h"
#include "Entity.hpp"


Entity::Entity(){
}

Entity::Entity(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics){
	this->physics = physics;
	this->renderer = renderer;
}

void Entity::addConstraint(btTypedConstraint* constraint, bool disableCollisionBetweenLinkedBodies) {
	physics->addConstraint(constraint, disableCollisionBetweenLinkedBodies);
	constraints.push_back(constraint);
}


Entity::~Entity(){
	for (auto& constraint : constraints) {
		physics->removeConstraint(constraint);
		delete constraint;
	}

	for (auto& part : parts)
		delete part;
}

void Entity::handleInput(float deltaTime)
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

void Entity::draw(){
	for (auto& part : parts)
		part->draw();
}
