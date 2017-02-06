#pragma once
#include "Shape.hpp"

#include <map>

class GenericShape :
	public Shape
{
public:
	GenericShape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, std::string filePath, glm::vec3 position, glm::vec3 scale, float mass, float yaw, float pitch, float roll);
	~GenericShape();
	static InstanceBuilder& initializeBuilder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, std::string filePath);
private:
	static std::map<std::string, InstanceBuilder> builders;
};

