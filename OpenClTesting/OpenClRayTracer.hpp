#pragma once

#include "ArraySlice.hpp"
#include "Containers.hpp"
#include "OpenGlShaders.hpp"

#include <vector>
#include <array>
#include "CL\cl2.hpp"
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include <chrono>
#include "bullet\BulletCollision\CollisionShapes\btOptimizedBvh.h"

#define ADVANCED_RENDERER
#define MAX_VERTICES_PER_OBJECT 5000000//320
#define MAX_INDICES_PER_OBJECT 10000000//640
#define MAX_OBJECTS_IN_RAY 16


#define RAY_DEPTH 2
typedef std::chrono::high_resolution_clock::time_point RtTimePoint;
typedef std::chrono::high_resolution_clock::duration RtDuration;

class OpenClRayTracer
{
public:
	OpenClRayTracer();
	OpenClRayTracer(int width, int height);
	~OpenClRayTracer();

	void clear();
	void push_back(Instance instance);
	void push_back(MultiInstance& instance);

	void push_backTexture(std::string path);

	Instance pop_instance();

	InstanceBuilder push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices);

	MultiInstanceBuilder push_backMultiObjectTypes(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, int maxVerticesPerObject = MAX_VERTICES_PER_OBJECT, int maxIndicesPerObject = MAX_INDICES_PER_OBJECT);

	//ArraySlice<TriangleIndices> getTriangles(Object object);	// Doing stuff to thise object type will alter every instance of this object type once the buffers are updated
	//ArraySlice<Vertex> getVertices(Object object);			// Not yet implemented

	InstanceBuilder push_backToObjectTypeBuffers(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices);

	MultiInstanceBuilder push_backMultiToObjectTypeBuffers(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, int maxVerticesPerObject = MAX_VERTICES_PER_OBJECT, int maxIndicesPerObject = MAX_INDICES_PER_OBJECT);

	void writeToObjectTypeBuffers();

	void writeToInstanceBuffer();

	

	void autoResize();
	void autoResizeObjectTypes();
	void reserve(int maxInstanceCount, int maxTotalVertexCount);

	void computeOnCPU();
	cl::Event vertexShaderNonBlocking();
	cl::Event aabbNonBlocking();
	cl::Event prepRayTraceNonBlocking();
	cl::Event rayTraceNonBlocking(float16 matrix);
	void debugCl();
	void rayTrace(float16 matrix);
	void draw();

	void compabilityDraw();

	void initializeAdvancedRender();

	GLFWwindow* getWindow() {return renderer.getWindow();}
	void render(float16 matrix);

	
	void resizeCallback(GLFWwindow * window, int width, int height);

private:
	void initialize();
	void reserveArrays(int maxInstanceCount);
	void reserveObjectTypeBuffers(int maxObjectTypeCount, int maxObjectTypeTriangleCount, int maxObjectTypeVertexCount);
	void reserveBuffers(int maxObjectCount, int maxVertexCount);
	void profileAdvancedRender(RtTimePoint startTime, std::vector<RtTimePoint> rayTracerStartTimes, std::vector<RtTimePoint> rayGeneratorStartTimes, std::vector<RtTimePoint> treeTraverserStartTimes, RtTimePoint colorToPixelStartTime, RtTimePoint drawingStartTime, RtTimePoint doneTime);
	double OpenClRayTracer::durationToMs(RtDuration duration);

	std::vector<Object> objectTypes;
#ifdef BVH
	std::vector<btOptimizedBvhNode> objectTypeTriangleBvhNodes;
#endif // BVH
	std::vector<TriangleIndices> objectTypeIndices;
	std::vector<Vertex> objectTypeVertices;
	std::vector<Instance> objectInstances;

	unsigned transformedVertexCount = 0;

	cl::Kernel vertexShaderKernel;
	cl::Kernel aabbKernel;
	cl::Kernel rayTraceKernel;
	cl::Kernel iterativeRayTracerKernel;
	cl::Kernel debugKernel;

	cl::Kernel perspectiveRayGeneratorKernel;
	cl::Kernel rayTraceAdvancedKernel;
	cl::Kernel rayGeneratorKernel;
	cl::Kernel treeTraverserKernel;
	cl::Kernel colorToPixelKernel;
	cl::Kernel colorToColorKernel;

	cl::Context context;
	cl::CommandQueue queue;




	cl::Buffer objectTypeBuffer;

#ifdef BVH
	cl::Buffer objectTypeTriangleBvhNodesBuffer;
#endif// BVH

	cl::Buffer objectTypeIndexBuffer;
	cl::Buffer objectTypeVertexBuffer;

	int objectTypeBufferCapacity = 0;
#ifdef BVH
	int objectTypeTriangleBvhNodesBufferCapacity = 0;
#endif// BVH
	int objectTypeIndexBufferCapacity = 0;
	int objectTypeVertexBufferCapacity = 0;

	cl::Buffer objectInstanceBuffer;
	cl::Buffer transformedObjectBuffer;
	cl::Buffer transformedVertexBuffer;

	int instanceBufferCapacity = 0;
	int transformedVertexBufferCapacity = 0;

	std::vector<cl::Buffer> rayBuffers;
	std::vector<cl::Buffer> rayTreeBuffers;
	std::vector<cl::Buffer> hitBuffers;

	std::vector<cl::Image2D> textures;


	bool useInterop = true;
	std::vector<cl::Memory> resultImages;// Has to be vector even though there is only one element due to queue.enqueueAcquireGLObjects

	cl::Buffer resultPixels;
	std::vector<float4> colors;


	OpenGlShaders renderer;
	GLuint openGlTextureID;

	int width;
	int height;

	
};