#pragma once

#include <vector>

#include "Shape.hpp"
#include "Input.hpp"

class Entity
{
public:
	Entity();
	Entity(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	void addConstraint(btTypedConstraint * constraint, bool disableCollisionBetweenLinkedBodies);
	~Entity();
	virtual void handleInput(const Input& input, float deltaTime);
	virtual glm::mat4 getTranslationMatrix();
	virtual glm::mat4 getMatrix();
	virtual glm::vec3 getPosition();
	virtual void update(float deltaTime);
	virtual void draw() const;
	virtual bool isVehicle() { return false; }
	float distance2(Entity* other);
	void setArrayIndex(int index) { gameArrayIndex = index; }
	int getArrayIndex() { return gameArrayIndex; }
protected:
	
	std::vector<Shape*> parts;
	btDiscreteDynamicsWorld* physics;
	ClRayTracer* renderer;
	std::vector<btTypedConstraint*> constraints;
	bool isServer();
private:
	int gameArrayIndex = -1;
	
};

