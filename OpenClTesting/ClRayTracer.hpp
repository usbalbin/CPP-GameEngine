#pragma once

#include "ArraySlice.hpp"
#include "Containers.hpp"
#include "OpenGlShaders.hpp"

#include "ClReadBuffer.hpp"
#include "MaterialManager.hpp"

#include <unordered_map>
#include <vector>
#include <array>
#include "CL\cl2.hpp"
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include <chrono>
#include "bullet\BulletCollision\CollisionShapes\btOptimizedBvh.h"


#define GPU_CONTEXT_ID 0			//GPU ID
#define GPU_DEVICE_ID 0
#define MAX_TEXTURE_COUNT 8

#define RAY_DEPTH 2
typedef std::chrono::high_resolution_clock::time_point RtTimePoint;
typedef std::chrono::high_resolution_clock::duration RtDuration;

class ClRayTracer
{
public:
	ClRayTracer();


	ClRayTracer(int width, int height, bool forceCpu, bool fullScreen);
	~ClRayTracer();

	void clear();
	void push_back(Instance instance);

	void push_back(MultiInstance & multiInstance);

	


	InstanceBuilder push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices);
	InstanceBuilder push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, const InstanceBuilder & builderWithCommonVertices);

	Instance makeInstanceOld(std::string meshPath, float16 initialTransform = float16(1));
	MultiInstance makeInstance(std::string meshPath, float16 initialTransform = float16(1));
	
	//The key indices is formated like "<MTL-path>:<materialName>"
	void readObjMulti(std::vector<Vertex>& vertices, std::unordered_map<std::string, std::vector<TriangleIndices>>& indices, std::string & filePath, float reflection = 0, float refraction = 0);
	
	void getMeshData(const Instance& instance, std::vector<TriangleIndices>& indicesOut, std::vector<Vertex>& verticesOut);
	void getMeshVertices(const Instance & instance, std::vector<Vertex>& verticesOut);
	void getMeshPoints(const Instance & instance, float *& const vertices, int & vertexCount, int & stride);

	//Only use me when first instance has every vertex of all of the others combined, such as is returned by makeInstance
	void getMeshPointsJoinedMulti(const MultiInstance & instance, float*& const vertices, int& vertexCount, int& stride);

	//ArraySlice<TriangleIndices> getTriangles(Object object);	// Doing stuff to thise object type will alter every instance of this object type once the buffers are updated
	//ArraySlice<Vertex> getVertices(Object object);			// Not yet implemented

	void debugCl();
	void draw();

	void compabilityDraw();

	void initializeAdvancedRender();

	GLFWwindow* getWindow() { return renderer.getWindow(); }
	void render(float16 matrix);

	void resizeCallback(GLFWwindow * window, int width, int height);

private:
	void initialize();
	void profileAdvancedRender(RtTimePoint startTime, std::vector<RtTimePoint> rayTracerStartTimes, std::vector<RtTimePoint> rayGeneratorStartTimes, std::vector<RtTimePoint> treeTraverserStartTimes, RtTimePoint colorToPixelStartTime, RtTimePoint drawingStartTime, RtTimePoint doneTime);
	double ClRayTracer::durationToMs(RtDuration duration);

	ClReadBuffer<Object> objectTypes;

	ClReadBuffer<btOptimizedBvhNode> objectTypeTriangleBvhNodes;

	ClReadBuffer<TriangleIndices> objectTypeIndices;
	ClReadBuffer<Vertex> objectTypeVertices;
	ClReadBuffer<Instance> objectInstances;

	unsigned transformedVertexCount = 0;

	cl::Kernel debugKernel;

	cl::Kernel perspectiveRayGeneratorKernel;
	cl::Kernel rayTraceAdvancedKernel;
	cl::Kernel rayGeneratorKernel;
	cl::Kernel treeTraverserKernel;
	cl::Kernel colorToPixelKernel;
	cl::Kernel colorToColorKernel;

	cl::Context context;
	cl::CommandQueue queue;


	std::vector<cl::Buffer> rayBuffers;
	std::vector<cl::Buffer> rayTreeBuffers;
	std::vector<cl::Buffer> hitBuffers;

	std::unordered_map<std::string, MultiInstanceBuilder> multiBuilders;
	std::unordered_map<std::string, InstanceBuilder> builders;
	MaterialManager materialManager;


	bool useInterop = true;
	std::vector<cl::Memory> resultImages;// Has to be vector even though there is only one element due to queue.enqueueAcquireGLObjects

	cl::Buffer resultPixels;
	std::vector<float4> colors;


	OpenGlShaders renderer;
	GLuint openGlTextureID;

	int width;
	int height;
	bool forceCpu;
	bool fullScreen;

};