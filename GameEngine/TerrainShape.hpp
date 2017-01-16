#pragma once
#include "Shape.hpp"

#include "BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

class TerrainShape :
	public Shape
{
public:
	TerrainShape(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	~TerrainShape();


	static void initializeBuilder(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics);

	static bool builderInitialized;
	static MultiInstanceBuilder graphicsObjectBuilder;
	static btHeightfieldTerrainShape* terrainShape;
	static float heightScale;
	static std::vector<float> heights;
};

