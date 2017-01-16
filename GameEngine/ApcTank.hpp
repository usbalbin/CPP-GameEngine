#pragma once
#include "Entity.hpp"
class ApcTank :
	public Entity
{
public:
	ApcTank(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	void handleInput(float deltaTime);
	~ApcTank();
};

