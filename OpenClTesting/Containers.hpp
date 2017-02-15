#pragma once
#define NOMINMAX
#include "glm\mat4x4.hpp"
#include "glm\gtx\norm.hpp"
#include "glm\vec2.hpp"
#include "glm\vec3.hpp"
#include "glm\vec4.hpp"

#include <algorithm>
#include <vector>

#define BVH



//Always use largest type first in structs shared by openCL device, also make sure they are defined AND aligned the same

typedef glm::mat4 float16;

typedef glm::vec2 float2;
typedef glm::vec3 float3;
typedef glm::vec4 float4;

bool operator<(float2 a, float2 b);
bool operator<(const float2& a, const float2& b);
bool operator<(float3 a, float3 b);
bool operator<(const float3& a, const float3& b);
bool operator<(float4 a, float4 b);
bool operator<(const float4& a, const float4& b);

/*
union Color {
	Color() {}
	Color(float3 color) : color(color, 0.0f), properties(0.0f, 0.0f, 1.0f, 0.0f) {}
	Color(float4 color) : color(color), properties(0.0f, 0.0f, 1.0f, 0.0f) {}
	Color(float3 color, float4 properties) : color(color, 0.0f), properties(properties) {}
	Color(float4 color, float4 properties) : color(color), properties(properties) {}
	Color(float3 color, float refraction, float reflection, float refractionIndex, float reflectiveDiffusion) : color(color, 0.0f) {
		this->refract = refraction;
		this->reflect = reflection;
		this->refractIndex = refractionIndex;
		this->reflectDuffusion = reflectiveDiffusion;
	}
	Color(float4 color, float refraction, float reflection, float refractionIndex, float reflectiveDiffusion) : color(color) {
		this->refract = refraction;
		this->reflect = reflection;
		this->refractIndex = refractionIndex;
		this->reflectDuffusion = reflectiveDiffusion;
	}


	struct {
		float4 color;
		float4 properties;
	};
	struct {
		float red; 
		float green;
		float blue;
		float padding;

		float refract;
		float reflect;
		float refractIndex;
		float reflectDuffusion;
	};
	
};

*/
/*
struct Light {
	float4 color;
	__declspec(align(4 * sizeof(float))) float3 position;
	__declspec(align(4 * sizeof(float))) float radius;
};*/

#define TEXTURE_COUNT 16

struct Vertex {
	Vertex() {}
	Vertex(float3 position, float4 color) : position(position), color(color), normal(0.0f), reflectFactor(0), refractFactor(0) {}
	Vertex(float3 position, float3 normal, float2 uv) : position(position), normal(normal), uv(uv), color(0.0f), reflectFactor(0), refractFactor(0) {}
	Vertex(float3 position, float4 color, float3 normal) : position(position), color(color), normal(normal), reflectFactor(0.5f), refractFactor(0.25f) {}
	Vertex(float3 position, float4 color, float3 normal, float reflectFactor, float refractFactor) : position(position), color(color), normal(normal), reflectFactor(reflectFactor), refractFactor(refractFactor) {}
	//Vertex(float3 position, unsigned char textures[TEXTURE_COUNT], float3 normal, float reflectFactor = 0, float refractFactor = 0) : position(position), normal(normal), reflectFactor(reflectFactor), refractFactor(refractFactor) { std::copy(&textures[0], &textures[TEXTURE_COUNT], this->textures); }
	//Vertex(float3 position, Color color, float3 normal) : position(position), color(color), normal(normal) {}
	float4 color;
	__declspec(align(4 * sizeof(float))) float3 normal;
	__declspec(align(4 * sizeof(float))) float3 position;
	__declspec(align(4 * sizeof(float))) float2 uv;
	float reflectFactor;
	float refractFactor;

	//bool operator<(const Vertex other);
};
bool operator<(Vertex, Vertex);
bool operator==(const Vertex&, const Vertex&);



struct Ray {
	__declspec(align(4 * sizeof(float))) float3 position;
	__declspec(align(4 * sizeof(float))) float3 direction;
	__declspec(align(4 * sizeof(float))) float3 inverseDirection;//inverseDirection = 1.0f / direction;
};


struct Triangle {
	Triangle() {}
	Triangle(Vertex a, Vertex b, Vertex c) : a(a), b(b), c(c){}
	Vertex a, b, c;
};

struct TriangleIndices {
	TriangleIndices() {}
	TriangleIndices(int a, int b, int c) : a(a), b(b), c(c) {}
	
	int& operator[](int index) { return values[index]; }

	union{
		int values[3];
		struct { int a, b, c; };
	};
};

struct AABB {
	AABB() {}
	AABB(const std::vector<Vertex>& vertices) {
		min = max = vertices[0].position;

		for (int i = 1; i < vertices.size(); i++) {
			min = float3(
				std::min<float>(min.x, vertices[i].position.x),
				std::min<float>(min.y, vertices[i].position.y),
				std::min<float>(min.z, vertices[i].position.z)
			);
			max = float3(
				std::max<float>(max.x, vertices[i].position.x),
				std::max<float>(max.y, vertices[i].position.y),
				std::max<float>(max.z, vertices[i].position.z)
			);
		}
	}
	AABB(std::vector<float3>& positions) {
		min = max = positions[0];

		for (int i = 1; i < positions.size(); i++) {
			min = float3(
				std::min(min.x, positions[i].x),
				std::min(min.y, positions[i].y),
				std::min(min.z, positions[i].z)
			);
			max = float3(
				std::max(max.x, positions[i].x),
				std::max(max.y, positions[i].y),
				std::max(max.z, positions[i].z)
			);
		}
	}
	AABB(float3 min, float3 max) : min(min), max(max) {}
	float3 center() { return float3((max.x + min.x)  * 0.5f, (max.y + min.y)  * 0.5f, (max.z + min.z) * 0.5f); }
	bool isInside(float3 point) {
		if (max.x < point.x || min.x > point.x) return false;
		if (max.y < point.y || min.y > point.y) return false;
		
		return !(max.z < point.z || min.z > point.z);
	}
	__declspec(align(4 * sizeof(float))) float3 min;
	__declspec(align(4 * sizeof(float))) float3 max;
};

struct Object {
	Object() {}
	AABB boundingBox;
	int startTriangle;
	int startVertex;
	int numTriangles;
	int numVertices;


	int bvhRootNodeIndex;
	int bvhTreeSize;
};

struct InstanceBuilder : public Object {
	InstanceBuilder() {};
	InstanceBuilder(Object object, int meshType) {

		this->boundingBox = object.boundingBox;
		this->startTriangle = object.startTriangle;
		this->startVertex = object.startVertex;
		this->numTriangles = object.numTriangles;
		this->numVertices = object.numVertices;
		this->meshType = meshType;
		this->texId = -1;
	}
	int meshType;
	size_t texId;
};

struct MultiInstanceBuilder {
	std::vector<InstanceBuilder> instanceBuilders;
};

struct Instance {
	Instance() : meshType(-1){};
	Instance(float16 modelMatrix, float16 invModelMatrix, InstanceBuilder builder) : modelMatrix(modelMatrix), invModelMatrix(invModelMatrix), meshType(builder.meshType), textureId(builder.texId) {};
	bool isInitialized() { return meshType != -1; }

	float16 modelMatrix;
	float16 invModelMatrix;
	void* broadPhaseNodePtr = nullptr;
	int meshType;
	int startVertex;
	
	int textureId;
	int padding[11];// = { 0 };
};

struct MultiInstance {
	MultiInstance() {}
	MultiInstance(float16& modelMatrix, MultiInstanceBuilder& builder) {
		auto inv = glm::inverse(modelMatrix);
		for (auto& instanceBuilder : builder.instanceBuilders)
			instances.emplace_back(modelMatrix, inv, instanceBuilder);
	}
	std::vector<Instance> instances;
	bool isInitialized() { return !instances.empty() && instances[0].isInitialized(); }
	float16 getMatrix() { return instances[0].modelMatrix; }
	void setMatrix(float16& matrix) {
		float16 invMatrix = glm::inverse(matrix);
		for (auto& instance : instances) {
			instance.modelMatrix = matrix;
			instance.invModelMatrix = invMatrix;
		}
	}
};

struct Hit {
	Vertex vertex;
	Ray ray;
};

struct RayTree {
	float4 color;
	int reflectIndex;
	int refractIndex;
	float reflectFactor;
	float refractFactor;
};