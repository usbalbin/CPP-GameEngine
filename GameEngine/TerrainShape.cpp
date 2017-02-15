#include "stdafx.h"
#include "TerrainShape.hpp"

bool TerrainShape::builderInitialized = false;
InstanceBuilder TerrainShape::graphicsObjectBuilder;
btHeightfieldTerrainShape* TerrainShape::terrainShape;
float TerrainShape::heightScale = 40;
std::vector<float> TerrainShape::heights;

TerrainShape::TerrainShape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics) : Shape(nullptr, renderer, physics){
	initializeBuilder(renderer, physics);
	
	this->scale = glm::vec3(4, heightScale, 4);
	glm::mat4 matrix =
		glm::scale(
			glm::mat4(1.0f),
			scale
		);

	Instance instance(matrix, glm::inverse(matrix), graphicsObjectBuilder);
	
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());

	btRigidBody* rigidBody = new btRigidBody(0, motionState, terrainShape);

	initialize(instance, rigidBody);
}

void TerrainShape::initializeBuilder(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics) {
	if (builderInitialized)//Only initialize one
		return;

	std::vector<TriangleIndices> indices;
	std::vector<Vertex> vertices;
	
	int imageWidth = 2, imageHeight = 2;

	std::vector<ubyte3> pixels = readBmpPixels(std::string("content/Terrain/heightmap2.bmp"), &imageWidth, &imageHeight);
	/*{
		ubyte3(0, 0, 0), ubyte3(0, 0, 0),
		ubyte3(0, 0, 0), ubyte3(1, 0, 0),
	};*/
	pixelsToMesh(imageWidth, imageHeight, pixels, vertices, indices);
	
	heights.reserve(imageHeight * imageWidth);
	
	for (auto& vertex : vertices) {
		heights.push_back(vertex.position.y);
	}

	graphicsObjectBuilder = renderer->push_backObjectType(indices, vertices);
	graphicsObjectBuilder.texId = renderer->getTextureId("content/Terrain/groundTexture.bmp");
	builderInitialized = true;

	
	terrainShape = new btHeightfieldTerrainShape(imageWidth, imageHeight, (void*)heights.data(), 1, -1, +1, 1, PHY_ScalarType::PHY_FLOAT, false);
	terrainShape->setLocalScaling(btVector3(4, heightScale, 4));
}

TerrainShape::~TerrainShape()
{
}
