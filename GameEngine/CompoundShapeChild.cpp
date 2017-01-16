#include "stdafx.h"
#include "CompoundShapeChild.hpp"

#include "Utils.hpp"

CompoundShapeChild::CompoundShapeChild(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, btTransform localTransform, Shape* shape, btRigidBody* parentCompoundBody){
	this->localTransform = localTransform;
	this->shape = shape;
	this->parentCompoundBody = parentCompoundBody;
	shape->disablePhysics();
}

CompoundShapeChild::~CompoundShapeChild()
{
}

void CompoundShapeChild::setParent(btRigidBody* parentCompoundBody) {
	this->parentCompoundBody = parentCompoundBody;
}

void CompoundShapeChild::draw(){
	glm::mat4 matrix = glm::scale(glm::mat4(1), shape->getScale()) * toMatrix(localTransform) * toMatrix(parentCompoundBody->getWorldTransform());// *glm::scale(toMatrix(localTransform), shape->getScale());
	
	shape->draw(matrix);
}

void CompoundShapeChild::update(float deltaTime){
	shape->update(deltaTime);
}

btRigidBody * CompoundShapeChild::getPhysicsObject()
{
	return shape->physicsObject;
}

btCollisionShape * CompoundShapeChild::getCollisionShape()
{
	return shape->getCollisionShape();
}

btTransform CompoundShapeChild::getLocalTransform()
{
	return localTransform;
}

glm::mat4 CompoundShapeChild::getWorldMatrix()
{
	return toMatrix(localTransform) * toMatrix(parentCompoundBody->getWorldTransform());
}

/*
 * This will not update this childs local position within the compound shape, it will only affect the graphical representetion.
 * Use parent compound shape's updateChildTransform to update to update this childs local position within the compound shape.
 */
void CompoundShapeChild::setLocalTransform(btTransform newTransform) {
	this->localTransform = newTransform;
}

Shape * CompoundShapeChild::getShape()
{
	return shape;
}
