#include "stdafx.h"
#include "GenericShape.hpp"
#include "Utils.hpp"

GenericShape::GenericShape(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, std::string meshPath, glm::vec3 position, glm::vec3 scale, float mass, float yaw, float pitch, float roll) : Shape(parent, renderer, physics){

	MultiInstance instance = renderer->makeInstance(meshPath);
	this->scale = scale;

	float* points;
	int pointCount, stride;
	renderer->getMeshPointsJoinedMulti(instance, points, pointCount, stride);
	btConvexHullShape* shape = new btConvexHullShape(points, pointCount, stride);
	shape->optimizeConvexHull();
	
	btTransform transform(btQuaternion(yaw, pitch, roll), toVector3(position));
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	btVector3 inertia;
	shape->setLocalScaling(toVector3(scale));
	shape->calculateLocalInertia(mass, inertia);
	btRigidBody* rigidBody = new btRigidBody(mass, motionState, shape, inertia);

	initialize(instance, rigidBody);
}


GenericShape::~GenericShape()
{
}