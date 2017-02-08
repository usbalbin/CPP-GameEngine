#pragma once
#include "Shape.hpp"


class Cube :
	public Shape
{
public:
	Cube();
	Cube(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), float mass = 0, float yaw = 0, float pitch = 0, float roll = 0);
	~Cube();


private:
	
};

