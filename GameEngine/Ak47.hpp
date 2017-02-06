#pragma once

#include "CompoundShape.hpp"
#include "Barrel.hpp"
#include "Input.hpp"

class Ak47 : public CompoundShape
{
public:
	Ak47(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);

	~Ak47();

	void draw() override;
	void update(float deltaTime) override;

	void handleInput(const Input& input, float deltaTime);




	static void initializeBuilder(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics);

	static bool builderInitialized;
	static InstanceBuilder graphicsObjectBuilder;

private:
	CompoundShapeChild* barrel;
};

