#pragma once
#include "Entity.hpp"
class Terrain :
	public Entity
{
public:
	Terrain(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics);
	~Terrain();
};

