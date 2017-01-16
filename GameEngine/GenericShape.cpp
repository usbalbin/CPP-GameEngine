#include "stdafx.h"
#include "GenericShape.hpp"

std::map<std::string, MultiInstanceBuilder> GenericShape::builders;

GenericShape::GenericShape(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, std::string filePath, glm::vec3 position, glm::vec3 scale, float mass, float yaw, float pitch, float roll) : Shape(renderer, physics){

}


GenericShape::~GenericShape()
{
}

MultiInstanceBuilder& GenericShape::initializeBuilder(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, std::string filePath) {
	if (builders.find(filePath) != builders.end())
		return builders[filePath];

	std::vector<TriangleIndices> indices;
	std::vector<Vertex> vertices;
	float reflection = 0.0f, refraction = 0.0f;

	readObjFile(vertices, indices, filePath, reflection, refraction);

	builders[filePath] = renderer->push_backMultiObjectTypes(indices, vertices);
	
	return builders[filePath];
}