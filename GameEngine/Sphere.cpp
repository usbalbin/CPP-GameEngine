#include "stdafx.h"
#include "Sphere.hpp"

#include "OpenClTesting\Meshes.hpp"

bool Sphere::builderInitialized = false;
InstanceBuilder Sphere::graphicsObjectBuilder;

Sphere::Sphere(){
}

Sphere::Sphere(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float scale, float mass, float yaw, float pitch, float roll)
	: Shape(parent, renderer, physics)
{
	glm::mat4 matrix =
		glm::translate(
			glm::mat4(1.0f),
			position
		)

		*

		glm::scale(
			glm::yawPitchRoll(yaw, pitch, roll),
			glm::vec3(scale)
		)
	;

	this->scale = glm::vec3(scale);

	if (!builderInitialized && renderer) {
		initializeBuilder(renderer, physics);
		//renderer->writeToObjectTypeBuffers();
	}
	Instance instance(matrix, glm::inverse(matrix), graphicsObjectBuilder);

	btSphereShape* sphereShape = new btSphereShape(scale);

	btTransform transform(btQuaternion(yaw, pitch, roll), btVector3(position.x, position.y, position.z));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia(0, 0, 0);
	sphereShape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, sphereShape, inertia);

	initialize(instance, rigidBody);

}

void Sphere::initializeBuilder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics) {
	if (builderInitialized)//Only initialize one
		return;

	float qualityFactor = 10.0f;
	glm::vec4 color(0, 0, 0, 1);
	
	std::vector<TriangleIndices> indices = genSphereIndices(qualityFactor);
	std::vector<Vertex> vertices = genSphereVertices(1.0f, color, qualityFactor);

	graphicsObjectBuilder = renderer->push_backObjectType(indices, vertices);
	builderInitialized = true;
}

Sphere::~Sphere() {

}

float Sphere::getRadius()
{
	return scale.x;
}
