#pragma once
#include "Shape.hpp"


class Sphere :
	public Shape
{
public:
	Sphere();
	Sphere(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position = glm::vec3(0.0f), float scale = 1.0f, float mass = 0, float yaw = 0, float pitch = 0, float roll = 0);
	~Sphere();
	float getRadius();

	static void initializeBuilder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);

	static bool builderInitialized;
	static InstanceBuilder graphicsObjectBuilder;
private:

};
