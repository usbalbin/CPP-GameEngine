#pragma once
#include "Shape.hpp"

#include "BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

class TerrainShape :
	public Shape
{
public:
	TerrainShape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	~TerrainShape();


	static void initializeBuilder(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);

	static bool builderInitialized;
	static InstanceBuilder graphicsObjectBuilder;
	static btHeightfieldTerrainShape* terrainShape;
	static float heightScale;
	static std::vector<float> heights;
};

