#pragma once

#include "Shape.hpp"

class CompoundShapeChild
{
public:
	CompoundShapeChild(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, btTransform localTransform, Shape * shape, btRigidBody * parentCompoundBody);
	~CompoundShapeChild();

	void setParent(btRigidBody* parentCompoundBody);

	void draw();
	void update(float deltaTime);

	btRigidBody* getPhysicsObject();
	btCollisionShape* getCollisionShape();

	btTransform getLocalTransform();
	glm::mat4 getWorldMatrix();

	/*
	 * This will not update this childs local position within the compound shape, it will only affect the graphical representetion.
	 * Use parent compound shape's updateChildTransform to update to update this childs local position within the compound shape.
	 */
	void setLocalTransform(btTransform newTransform);
	Shape* getShape();
private:
	Shape* shape;
	btTransform localTransform;
	btRigidBody* parentCompoundBody;
};

