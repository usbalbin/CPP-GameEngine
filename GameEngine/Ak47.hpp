#pragma once

#include "CompoundShape.hpp"
#include "Barrel.hpp"
#include "Input.hpp"

class Ak47 : public CompoundShape
{
public:
	Ak47(Entity* parent, ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);

	~Ak47();

	void draw() override;
	void update(float deltaTime) override;

	void handleInput(const Input& input, float deltaTime);



private:
	CompoundShapeChild* barrel;
};

