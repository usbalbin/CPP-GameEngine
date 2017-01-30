#include "stdafx.h"
#include "TerrainShape.hpp"

bool TerrainShape::builderInitialized = false;
MultiInstanceBuilder TerrainShape::graphicsObjectBuilder;
btHeightfieldTerrainShape* TerrainShape::terrainShape;
float TerrainShape::heightScale = 10;
std::vector<float> TerrainShape::heights;

TerrainShape::TerrainShape(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics){
	this->scale = glm::vec3(1, heightScale, 1);
	glm::mat4 matrix =
		glm::scale(
			glm::mat4(1.0f),
			scale
		)
		;

	if (!builderInitialized) {
		initializeBuilder(renderer, physics);
		renderer->writeToObjectTypeBuffers();
	}
	MultiInstance instance(matrix, graphicsObjectBuilder);
	for(auto& ins : instance.instances)
	ins.texture[0] = SHRT_MAX;
	
	
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());

	btRigidBody* rigidBody = new btRigidBody(0, motionState, terrainShape);

	initialize(instance, rigidBody);
}

void TerrainShape::initializeBuilder(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics) {
	if (builderInitialized)//Only initialize one
		return;

	std::vector<TriangleIndices> indices;
	std::vector<Vertex> vertices;
	
	int imageWidth = 2, imageHeight = 2;

	std::vector<ubyte3> pixels = readBmpPixels(std::string("content/heightmap2.bmp"), &imageWidth, &imageHeight);
	/*{
		ubyte3(0, 0, 0), ubyte3(0, 0, 0),
		ubyte3(0, 0, 0), ubyte3(1, 0, 0),
	};*/
	pixelsToMesh(imageWidth, imageHeight, pixels, vertices, indices);
	
	heights.reserve(imageHeight * imageWidth);
	
	for (auto& vertex : vertices) {
		heights.push_back(vertex.position.y * heightScale);
	}

	graphicsObjectBuilder = renderer->push_backMultiToObjectTypeBuffers(indices, vertices);
	builderInitialized = true;

	
	terrainShape = new btHeightfieldTerrainShape(imageWidth, imageHeight, (void*)heights.data(), heightScale, -heightScale, +heightScale, 1, PHY_ScalarType::PHY_FLOAT, false);
}

TerrainShape::~TerrainShape()
{
}
