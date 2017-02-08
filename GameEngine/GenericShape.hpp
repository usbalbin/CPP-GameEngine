#pragma once
#include "Shape.hpp"

#include <map>

class GenericShape :
	public Shape
{
public:
	GenericShape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, std::string filePath, glm::vec3 position, float mass = 0, float yaw = 0, float pitch = 0, float roll = 0);
	~GenericShape();
private:
};

