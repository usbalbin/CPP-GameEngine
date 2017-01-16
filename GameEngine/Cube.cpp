#include "stdafx.h"
#include "Cube.hpp"



bool Cube::builderInitialized = false;
MultiInstanceBuilder Cube::graphicsObjectBuilder;

Cube::Cube(){
}

Cube::Cube(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec3 scale, float mass, float yaw, float pitch, float roll)
	: Shape(renderer, physics) {
	glm::mat4 matrix = 
		glm::translate(glm::mat4(1.0f), position)
		
		*
		
		glm::scale(
			glm::yawPitchRoll(yaw, pitch, roll),
			scale
		)
	;
	this->scale = scale;

	if (!builderInitialized) {
		initializeBuilder(renderer, physics);
		renderer->writeToObjectTypeBuffers();
	}
	MultiInstance instance(matrix, graphicsObjectBuilder);

	btBoxShape* boxShape = new btBoxShape(btVector3(scale.x, scale.y, scale.z));
	
	btTransform transform(btQuaternion(yaw, pitch, roll), btVector3(position.x, position.y, position.z));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia;
	boxShape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, boxShape, inertia);

	initialize(instance, rigidBody);
	
}

void Cube::initializeBuilder(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics){
	if (builderInitialized)//Only initialize one
		return;

	std::vector<TriangleIndices> indices;
	std::vector<Vertex> vertices;
	float reflection = 0.0f, refraction = 0.0f;

	readObjFile(vertices, indices, std::string("content/cube.obj"), reflection, refraction);

	graphicsObjectBuilder = renderer->push_backMultiObjectTypes(indices, vertices);
	builderInitialized = true;
}

Cube::~Cube(){
	
}
