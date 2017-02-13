#include "stdafx.h"
#include "Cube.hpp"




Cube::Cube(){
}

Cube::Cube(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale, float mass, float yaw, float pitch, float roll)
	: Shape(parent, renderer, physics) {
	glm::mat4 matrix = 
		glm::translate(glm::mat4(1.0f), position)
		
		*
		
		glm::scale(
			glm::yawPitchRoll(yaw, pitch, roll),
			scale
		)
	;
	this->scale = scale;

	
	Instance instance = renderer->makeInstanceSingleMaterial("content/basicObjects/cube.obj", float16(1), glm::vec4(0, 0, 0, 1), 1.0f);

	btBoxShape* boxShape = new btBoxShape(btVector3(scale.x, scale.y, scale.z));
	
	btTransform transform(btQuaternion(yaw, pitch, roll), btVector3(position.x, position.y, position.z));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia;
	boxShape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, boxShape, inertia);

	initialize(instance, rigidBody);
	
}


Cube::~Cube(){
	
}
