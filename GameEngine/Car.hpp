#pragma once
#include "Entity.hpp"


class Car :
	public Entity
{
public:
	Car(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	void handleInput(float deltaTime);
	~Car();
};

