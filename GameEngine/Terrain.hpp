#pragma once
#include "Entity.hpp"
class Terrain :
	public Entity
{
public:
	Terrain(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics);
	~Terrain();
};

