#include "stdafx.h"
#include "Cylinder.hpp"


bool Cylinder::builderInitialized = false;
MultiInstanceBuilder Cylinder::graphicsObjectBuilder;

Cylinder::Cylinder() {
}

Cylinder::Cylinder(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec2 size, float mass, float yaw, float pitch, float roll)
	: Shape(renderer, physics) {
	this->scale = glm::vec3(size, size.x);
	glm::mat4 matrix =
		glm::translate(glm::mat4(1.0f), position)

		*

		glm::scale(
			glm::yawPitchRoll(yaw, pitch, roll),
			scale
		)
		;
	

	if (!builderInitialized && renderer) {
		initializeBuilder(renderer, physics);
		renderer->writeToObjectTypeBuffers();
	}
	MultiInstance instance(matrix, graphicsObjectBuilder);

	btCylinderShape* boxShape = new btCylinderShape(btVector3(scale.x, scale.y, scale.z));

	btTransform transform(btQuaternion(yaw, pitch, roll), btVector3(position.x, position.y, position.z));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia;
	boxShape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, boxShape, inertia);

	initialize(instance, rigidBody);

}

void Cylinder::initializeBuilder(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics) {
	if (builderInitialized)//Only initialize one
		return;

	std::vector<TriangleIndices> indices;
	std::vector<Vertex> vertices;
	float reflection = 0.0f, refraction = 0.0f;

	readObjFile(vertices, indices, std::string("content/cylinder.obj"), reflection, refraction);

	graphicsObjectBuilder = renderer->push_backMultiObjectTypes(indices, vertices);
	builderInitialized = true;
}

Cylinder::~Cylinder() {

}
