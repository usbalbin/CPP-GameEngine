#pragma once
#include "Entity.hpp"
class Ball :
	public Entity
{
public:
	Ball(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float scale, float mass);
	~Ball();
};

