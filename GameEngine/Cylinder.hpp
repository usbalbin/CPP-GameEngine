#pragma once
#include "Shape.hpp"

class Cylinder :
	public Shape
{
public:
	Cylinder();
	Cylinder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, glm::vec2 scale, float mass, float yaw, float pitch, float roll);
	~Cylinder();

	static void initializeBuilder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	
	static bool builderInitialized;
	static InstanceBuilder graphicsObjectBuilder;
private:

};

