#include "stdafx.h"
#include "CompoundShape.hpp"

#include "Utils.hpp"
#include <numeric>


CompoundShape::CompoundShape(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics)
{
	this->renderer = renderer;
	this->physics = physics;
	physicsShape = new btCompoundShape();
}

CompoundShape::~CompoundShape() {
	for (auto& child : childShapes)
		delete child;
}

void CompoundShape::addChild(CompoundShapeChild* child) {
	childShapes.push_back(child);
	physicsShape->addChildShape(child->getLocalTransform(), child->getCollisionShape());
}

void CompoundShape::finalize(glm::vec3 position, std::vector<float> masses, float yaw, float pitch, float roll) {
	btDefaultMotionState* motionState = new btDefaultMotionState(toTransform(position, yaw, pitch, roll));

	btVector3 inertia;
	physicsShape->calculateLocalInertia(std::accumulate(masses.begin(), masses.end(), 0.0f), inertia);
	btTransform principal;
	physicsShape->calculatePrincipalAxisTransform(masses.data(), principal, inertia);

	for (int i = 0; i < childShapes.size(); i++) {
		btTransform newChildTransform = principal.inverse() * physicsShape->getChildTransform(i);
		physicsShape->updateChildTransform(i, newChildTransform);
		childShapes[i]->setLocalTransform(newChildTransform);
	}

	btRigidBody* rigidBody = new btRigidBody(std::accumulate(masses.begin(), masses.end(), 0.0f), motionState, physicsShape, inertia);
	this->physicsObject = rigidBody;
	physics->addRigidBody(rigidBody);

	for (auto child : childShapes)
		child->setParent(rigidBody);
}


void CompoundShape::draw(){
	for (auto child : childShapes)
		child->draw();
}

void CompoundShape::update(float deltaTime){
	for (auto part : childShapes)
		part->update(deltaTime);
}
