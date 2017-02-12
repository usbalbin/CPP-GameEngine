#include "stdafx.h"
#include "Cylinder.hpp"


Cylinder::Cylinder() {
}

Cylinder::Cylinder(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec2 size, float mass, float yaw, float pitch, float roll)
	: Shape(parent, renderer, physics) {
	this->scale = glm::vec3(size, size.x);
	glm::mat4 matrix =
		glm::translate(glm::mat4(1.0f), position)

		*

		glm::scale(
			glm::yawPitchRoll(yaw, pitch, roll),
			scale
		)
		;
	

	
	Instance instance = renderer->makeInstanceOld("content/basicObjects/cylinder.obj");

	btCylinderShape* boxShape = new btCylinderShape(btVector3(scale.x, scale.y, scale.z));

	btTransform transform(btQuaternion(yaw, pitch, roll), btVector3(position.x, position.y, position.z));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia;
	boxShape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, boxShape, inertia);

	initialize(instance, rigidBody);

}

Cylinder::~Cylinder() {

}
