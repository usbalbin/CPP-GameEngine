#pragma once
#include "Entity.hpp"
class Vehicle :
	public Entity
{
public:
	Vehicle(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics);
	~Vehicle();

	bool isVehicle() { return true; }
};

