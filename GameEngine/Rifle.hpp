#pragma once
#include "Shape.hpp"

#include "CompoundShape.hpp"
#include "Barrel.hpp"

class Rifle :
	public CompoundShape
{
public:
	Rifle(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw, float pitch, float roll);
	~Rifle();
	void update(float deltaTime);

private:
	CompoundShapeChild* barrel;
};

