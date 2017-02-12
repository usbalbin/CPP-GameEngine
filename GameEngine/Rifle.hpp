#pragma once
#include "Shape.hpp"

#include "CompoundShape.hpp"
#include "Barrel.hpp"
#include "Input.hpp"

class Rifle :
	public CompoundShape
{
public:
	Rifle(Entity* parent, ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw, float pitch, float roll);
	~Rifle();
	void update(float deltaTime);

	void handleInput(const Input& input, float deltaTime);

private:
	CompoundShapeChild* barrel;
};

