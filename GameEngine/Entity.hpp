#pragma once

#include <vector>

#include "Shape.hpp"

class Entity
{
public:
	Entity();
	Entity(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	void addConstraint(btTypedConstraint * constraint, bool disableCollisionBetweenLinkedBodies);
	~Entity();
	virtual void handleInput(float deltaTime);
	virtual glm::mat4 getTranslationMatrix();
	virtual glm::mat4 getMatrix();
	virtual glm::vec3 getPosition();
	virtual void update(float deltaTime);
	virtual void draw();
	virtual bool isVehicle() { return false; }
protected:
	std::vector<Shape*> parts;
	btDiscreteDynamicsWorld* physics;
	OpenClRayTracer* renderer;
	std::vector<btTypedConstraint*> constraints;
private:
};

