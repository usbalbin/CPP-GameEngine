#pragma once
#include "Vehicle.hpp"
#include "Cube.hpp"
#include "Barrel.hpp"

class Tank :
	public Vehicle
{
public:
	Tank(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);

	
	
	void handleInput(float deltaTime);
	void handleTurretInput(float deltaTime);
	void update(float deltaTime);
	~Tank();
private:
	Barrel* cannon;
	std::vector<btHinge2Constraint*> wheelConstraints;
	void setupTurret(glm::vec3 position, float yaw, float pitch, float roll, Cube* body, glm::vec3 bodyHalfExtents);

	btHinge2Constraint *turretPitchConstraint, *turretYawConstraint;
	float turretPitch = 0, turretYaw = 0;
};

