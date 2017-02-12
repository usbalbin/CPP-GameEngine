#pragma once
#include "Vehicle.hpp"
class HMMWV :
	public Vehicle
{
public:
	HMMWV(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	void handleInput(const Input& input, float deltaTime) override;
	~HMMWV();
};

