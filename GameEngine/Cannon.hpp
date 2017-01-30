#pragma once
#include "Entity.hpp"
class Cannon :
	public Entity
{
public:
	Cannon(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float mass);
	~Cannon();

	void handleInput(const Input& input, float deltaTime) override;
};

