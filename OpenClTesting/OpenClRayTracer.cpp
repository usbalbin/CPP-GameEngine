#include "stdafx.h"

#include "OpenClRayTracer.hpp"
#include "OpenClContexts.hpp"
#include "DebugRayTracer.hpp"
#include "Utils.hpp"
#include "BvhTree.hpp"

#include "GLFW\glfw3.h"

#include <iostream>
#include <stack>
#include <atomic>
#include <iomanip>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif // _WIN32

OpenClRayTracer::OpenClRayTracer(int width, int height) : 
	renderer(width, height, "shaders/vertexShader.glsl", "shaders/fragmentShader.glsl",
		[](GLFWwindow* window, int width, int height) {
			OpenClRayTracer* rayTracer = (OpenClRayTracer*)glfwGetWindowUserPointer(window);

			rayTracer->resizeCallback(window, width, height);
		}) {
	
	glfwSetWindowUserPointer(renderer.getWindow(), this);

	this->width = width;
	this->height = height;
	this->resultImages.resize(1);
	this->openGlTextureID = renderer.setupScreen();
	initialize();
}

OpenClRayTracer::~OpenClRayTracer()
{
}

void OpenClRayTracer::initialize() {
	OpenClContexts openClContexts;
	
	cl::Device device;
#ifndef FORCE_CPU
	if (useInterop) {
		try {
			openClContexts.initializeInteropGpu();
			device = openClContexts.getGpuDevice(0);
			this->context = openClContexts.getGpuContext(0);
		}
		catch (cl::Error e) {
			useInterop = false;
		}
	}
	if (!useInterop) {
		try {
			openClContexts.initializeGpu();
			openClContexts.getGpuDevice(0);
			this->context = openClContexts.getGpuContext(0);
		}
		catch (cl::Error e) {
			openClContexts.initializeCpu();
			device = openClContexts.getCpuDevice(0);
			this->context = openClContexts.getCpuContext(0);
		}
	}
#else
	openClContexts.initializeCpu();
	device = openClContexts.getCpuDevice(0);
	this->context = openClContexts.getCpuContext(0);
	useInterop = false;
#endif
	


	cl::Program::Sources sources;

	std::string vertexShaderSource = readFileToString("kernels/vertexShader.cl");
	std::string aabbSource = readFileToString("kernels/aabb.cl");
	std::string rayTracerSource = readFileToString("kernels/oldKernels/rayTracer.cl");
	std::string rayTracerKernelSource = readFileToString("kernels/oldKernels/rayTracerMain.cl");
	std::string debugSource = readFileToString("kernels/debug.cl");
#ifdef ADVANCED_RENDERER
	std::string perspectiveRayGeneratorSource = readFileToString("kernels/newKernels/1_perspectiveRayGenerator.cl");
	std::string rayGeneratorSource = readFileToString("kernels/newKernels/2A_rayGenerator.cl");
	std::string rayTraceAdvancedSource = readFileToString("kernels/newKernels/2B_rayTracer.cl");
	std::string treeTraverserSource = readFileToString("kernels/newKernels/3_treeTraverser.cl");
	std::string colorToPixelSource = readFileToString("kernels/newKernels/4_colorToPixel.cl");
#endif



	sources.push_back({ vertexShaderSource.c_str(), vertexShaderSource.length() });
	sources.push_back({ aabbSource.c_str(), aabbSource.length() });
	//sources.push_back({ rayTracerSource.c_str(), rayTracerSource.length() });
	sources.push_back({ rayTracerKernelSource.c_str(), rayTracerKernelSource.length() });
	sources.push_back({ debugSource.c_str(), debugSource.length() });
#ifdef ADVANCED_RENDERER
	sources.push_back({ perspectiveRayGeneratorSource.c_str(), perspectiveRayGeneratorSource.length() });
	sources.push_back({ rayTraceAdvancedSource.c_str(), rayTraceAdvancedSource.length() });
	sources.push_back({ rayGeneratorSource.c_str(), rayGeneratorSource.length() });
	sources.push_back({ treeTraverserSource.c_str(), treeTraverserSource.length() });
	sources.push_back({ colorToPixelSource.c_str(), colorToPixelSource.length() });
#endif

	writeSourcesToFile(sources, "kernels/output/allKernels.cl");

	cl::Program program(context, sources);



	std::cout << "---------------- Compilation status ----------------" << std::endl;
	std::string compileMessage;
	char programPathBuffer[256];
	getcwd(programPathBuffer, 256);
	std::string programPath = programPathBuffer;
	std::string stuff = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
	std::string supported_extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
	std::cout << supported_extensions << std::endl;


	std::cout << "Path: \"" << programPath << "\"" << std::endl;







	// KOLLA PÅ						-CL-STD=CL2.0












#ifdef ADVANCED_RENDERER
	std::string extraOptions = ""/*= "-cl-std=CL1.1 "*//*-s \"" + programPath + "kernels/newKernels/3_treeTraverser.cl\" "*/;
	extraOptions += "-cl-unsafe-math-optimizations -cl-fast-relaxed-math";
#else
	std::string extraOptions = "";// "-cl-std=CL2.0";// "-cl-std=c++";// "-cl-std=CL2.0";// "-cl-unsafe-math-optimizations -cl-fast-relaxed-math";
#endif
	std::string defines = " -D MAX_VERTICES_PER_OBJECT=" + std::to_string(MAX_VERTICES_PER_OBJECT);
	defines += " -D MAX_INDICES_PER_OBJECT=" + std::to_string(MAX_INDICES_PER_OBJECT);
	defines += " -D MAX_OBJECTS_IN_RAY=" + std::to_string(MAX_OBJECTS_IN_RAY);
#ifdef BVH
	defines += " -D BVH ";
#endif // BVH



	std::string compilerFlags = /*-O0 -g*/" -I " + programPath + " " + extraOptions + defines;
	std::cout << compilerFlags << std::endl;
	try {
		std::cout << "Build started..." << std::endl;
		program.build({ device }, compilerFlags.c_str());
	}catch(cl::Error e){
		glfwDestroyWindow(renderer.getWindow());
		std::cout << "Prepping error message..." << std::endl;
		compileMessage = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << "Failed to compile with status " << e.err() << ": " << compileMessage << std::endl;
		system("pause");
		exit(1);
	}
	
	compileMessage = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	std::cout << compileMessage << std::endl;
	
	cl_int status = CL_SUCCESS;
	
	queue = cl::CommandQueue(context, device, NULL, &status);
	

	
	vertexShaderKernel = cl::Kernel(program, "vertexShader", &status);
	aabbKernel = cl::Kernel(program, "aabb", &status);
	rayTraceKernel = cl::Kernel(program, "rayTracer", &status);
	debugKernel = cl::Kernel(program, "debug", &status);


#ifdef ADVANCED_RENDERER
	perspectiveRayGeneratorKernel = cl::Kernel(program, "perspectiveRayGenerator", &status);
	rayTraceAdvancedKernel = cl::Kernel(program, "rayTraceAdvanced", &status);
	rayGeneratorKernel = cl::Kernel(program, "rayGenerator", &status);
	treeTraverserKernel = cl::Kernel(program, "treeTraverser", &status);
	colorToPixelKernel = cl::Kernel(program, "colorToPixel", &status);
	colorToColorKernel = cl::Kernel(program, "colorToColor", &status);
#endif


	if (status != CL_SUCCESS) {
		std::cout << "Failed to create kernels" << std::endl;
		exit(1);
	}

	
	if (useInterop) {
		resultImages[0] = cl::ImageGL(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, openGlTextureID, &status);
		if (status != CL_SUCCESS) {
			std::cout << "Failed to create OpenCL image from OpenGL texture" << std::endl;
			exit(1);
		}
	}
	else {
		resultPixels = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float4) * width * height);
		colors = std::vector<float4>(width * height);
	}


	


}

void OpenClRayTracer::autoResize() {
	reserveArrays(objectInstances.size());
	reserveBuffers(objectInstances.size(), transformedVertexCount);
}

void OpenClRayTracer::autoResizeObjectTypes() {
	reserveObjectTypeBuffers(objectTypes.size(), objectTypeIndices.size(), objectTypeVertices.size());
}

void OpenClRayTracer::reserve(int maxInstanceCount, int maxTotalVertexCount) {
	reserveArrays(maxInstanceCount);
	reserveBuffers(maxInstanceCount, maxTotalVertexCount);
}

void OpenClRayTracer::reserveArrays(int maxInstanceCount) {
	this->objectInstances.reserve(maxInstanceCount);
}

void OpenClRayTracer::reserveObjectTypeBuffers(int maxObjectTypeCount, int maxObjectTypeTriangleCount, int maxObjectTypeVertexCount) {
	if (maxObjectTypeCount > objectTypeBufferCapacity) {
		objectTypeBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Object) * maxObjectTypeCount);
		objectTypeBufferCapacity = maxObjectTypeCount;
	}

#ifdef BVH
	if ((2 * maxObjectTypeTriangleCount) > objectTypeTriangleBvhNodesBufferCapacity) {
		objectTypeTriangleBvhNodesBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Object) * (2 * maxObjectTypeTriangleCount));
		objectTypeTriangleBvhNodesBufferCapacity = (2 * maxObjectTypeTriangleCount);
	}
#endif

	if (maxObjectTypeTriangleCount > objectTypeIndexBufferCapacity) {
		objectTypeIndexBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(TriangleIndices) * maxObjectTypeTriangleCount);
		objectTypeIndexBufferCapacity = maxObjectTypeTriangleCount;
	}

	if (maxObjectTypeVertexCount > objectTypeVertexBufferCapacity) {
		objectTypeVertexBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Vertex) * maxObjectTypeVertexCount);
		objectTypeVertexBufferCapacity = maxObjectTypeVertexCount;
	}
}

void OpenClRayTracer::reserveBuffers(int maxInstanceCount, int maxTotalVertexCount) {
	if (maxInstanceCount > instanceBufferCapacity) {
		objectInstanceBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Instance) * maxInstanceCount);
		transformedObjectBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Object) * maxInstanceCount);
		
		instanceBufferCapacity = maxInstanceCount;
	}
	if (maxTotalVertexCount > transformedVertexBufferCapacity) {
		transformedVertexBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Vertex) * maxTotalVertexCount);
		transformedVertexBufferCapacity = maxTotalVertexCount;
	}

}

void OpenClRayTracer::clear() {
	this->objectInstances.clear();
	this->transformedVertexCount = 0;
}

void OpenClRayTracer::push_back(Instance instance) {

#ifdef BVH
	instance.modelMatrix = glm::inverse(instance.modelMatrix);
#endif // BVH


	instance.startVertex = this->transformedVertexCount;

	Object objectType = this->objectTypes[instance.meshType];

	this->objectInstances.push_back(instance);
	this->transformedVertexCount += objectType.numVertices;
}

void OpenClRayTracer::push_back(MultiInstance& multiInstance)
{
	for (auto& instance : multiInstance.instances)
		push_back(instance);
}

Instance OpenClRayTracer::pop_instance() {
	Instance instance = this->objectInstances.back();

	instance.startVertex = -1;

	Object objectType = this->objectTypes[instance.meshType];
	this->transformedVertexCount -= objectType.numVertices;
	this->objectInstances.pop_back();

	return instance;
}

InstanceBuilder OpenClRayTracer::push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices) {
	Object objectType;

	
#ifdef BVH
	BvhTree bvhTree(objectTypeIndices, objectTypeVertices);

	objectType.bvhRootNodeIndex = this->objectTypeTriangleBvhNodes.size();
	objectType.bvhTreeSize = bvhTree.getCurNodeIndex();
	auto& t = bvhTree.getNodes();
	btOptimizedBvhNode currentNode;
	auto s = (char*)(&currentNode.m_triangleIndex) - (char*)&currentNode;
	bvhTree.appendNodesToVector(objectTypeTriangleBvhNodes);
#endif
	
	
	
	//objectType.boundingBox = AABB(float3(-5), float3(5)); //TODO remove this line!!!
	objectType.startTriangle = this->objectTypeIndices.size();
	objectType.startVertex = this->objectTypeVertices.size();
	objectType.numTriangles = objectTypeIndices.size();
	objectType.numVertices = objectTypeVertices.size();

	this->objectTypes.push_back(objectType);
	this->objectTypeIndices.insert(std::end(this->objectTypeIndices), std::begin(objectTypeIndices), std::end(objectTypeIndices));
	this->objectTypeVertices.insert(std::end(this->objectTypeVertices), std::begin(objectTypeVertices), std::end(objectTypeVertices));

	const int meshType = objectTypes.size() - 1;
	InstanceBuilder instanceBuilder(objectType, meshType);




	return instanceBuilder;
}

MultiInstanceBuilder OpenClRayTracer::push_backMultiObjectTypes(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, int maxVerticesPerObject, int maxIndicesPerObject)
{
	MultiInstanceBuilder multiInstanceBuilder;
	auto& instanceBuilders = multiInstanceBuilder.instanceBuilders;
	std::vector<std::vector<Vertex>> verticesCollections;
	std::vector<std::vector<TriangleIndices>> indicesCollections;

	verticesCollections.push_back(objectTypeVertices);
	indicesCollections.push_back(objectTypeIndices);
	

	for (int i = 0; i < verticesCollections.size();) {
		if (indicesCollections[i].size() < maxIndicesPerObject && verticesCollections[i].size() < maxVerticesPerObject) {//If small enough
			instanceBuilders.push_back(
				push_backObjectType(indicesCollections[i], verticesCollections[i])
			);
			i++;
		}
		else {
			verticesCollections.emplace_back();
			indicesCollections.emplace_back();
			splitMesh(verticesCollections[i], indicesCollections[i], verticesCollections.back(), indicesCollections.back());
		}
	}
	return multiInstanceBuilder;
}


InstanceBuilder OpenClRayTracer::push_backToObjectTypeBuffers(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices) {
	size_t objectTypesStart = objectTypes.size();
#ifdef BVH
	size_t objectTypeTriangleBvhNodesStart = objectTypeTriangleBvhNodes.size();
#endif // BVH	
	size_t objectTypeIndicesStart = objectTypeIndices.size();
	size_t objectTypeVerticesStart = objectTypeVertices.size();

	InstanceBuilder result = push_backObjectType(objectTypeIndices, objectTypeVertices);
	if (objectTypeBufferCapacity < objectTypes.size() || 
#ifdef BVH
		objectTypeTriangleBvhNodesBufferCapacity < objectTypeTriangleBvhNodes.size() ||
#endif // BVH
		objectTypeIndexBufferCapacity < objectTypeIndices.size() || objectTypeVertexBufferCapacity < objectTypeVertices.size()) {

		writeToObjectTypeBuffers();
		return result;
	}

	size_t objectTypesSize = objectTypes.size() - objectTypesStart;
#ifdef BVH
	size_t objectTypeTriangleBvhNodesSize = objectTypeTriangleBvhNodes.size() - objectTypeTriangleBvhNodesStart;
#endif // BVH	
	size_t objectTypeIndicesSize = objectTypeIndices.size() - objectTypeIndicesStart;
	size_t objectTypeVerticesSize = objectTypeVertices.size() - objectTypeVerticesStart;


	autoResizeObjectTypes();
	if (queue.enqueueWriteBuffer(objectTypeBuffer, CL_TRUE, sizeof(Object) * objectTypesStart, sizeof(Object) * objectTypesSize,&objectTypes[objectTypesStart]) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}
#ifdef BVH
	if (queue.enqueueWriteBuffer(objectTypeTriangleBvhNodesBuffer, CL_TRUE, sizeof(btOptimizedBvhNode) * objectTypeTriangleBvhNodesStart, sizeof(btOptimizedBvhNode) * objectTypeTriangleBvhNodesSize, &objectTypes[objectTypeTriangleBvhNodesStart]) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
}
#endif // BVH

	

	if (queue.enqueueWriteBuffer(objectTypeIndexBuffer, CL_TRUE, sizeof(TriangleIndices) * objectTypeIndicesStart, sizeof(TriangleIndices) * objectTypeIndicesSize,&objectTypeIndices[objectTypeIndicesStart]) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}

	if (queue.enqueueWriteBuffer(objectTypeVertexBuffer, CL_TRUE, sizeof(Vertex) * objectTypeVerticesStart, sizeof(Vertex) * objectTypeVerticesSize, &objectTypeVertices[objectTypeVerticesStart]) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}

	return result;
}

MultiInstanceBuilder OpenClRayTracer::push_backMultiToObjectTypeBuffers(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, int maxVerticesPerObject, int maxIndicesPerObject) {
	MultiInstanceBuilder result = push_backMultiObjectTypes(objectTypeIndices, objectTypeVertices, maxVerticesPerObject, maxIndicesPerObject);
	writeToObjectTypeBuffers();
	return result;
}

void OpenClRayTracer::writeToObjectTypeBuffers() {
	autoResizeObjectTypes();
	if (queue.enqueueWriteBuffer(objectTypeBuffer, CL_TRUE, 0, sizeof(Object) * objectTypes.size(), objectTypes.data()) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}

#ifdef BVH
	if (queue.enqueueWriteBuffer(objectTypeTriangleBvhNodesBuffer, CL_TRUE, 0, sizeof(btOptimizedBvhNode) * objectTypeTriangleBvhNodes.size(), objectTypeTriangleBvhNodes.data()) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
}
#endif // BVH

	

	if (queue.enqueueWriteBuffer(objectTypeIndexBuffer, CL_TRUE, 0, sizeof(TriangleIndices) * objectTypeIndices.size(), objectTypeIndices.data()) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}

	if (queue.enqueueWriteBuffer(objectTypeVertexBuffer, CL_TRUE, 0, sizeof(Vertex) * objectTypeVertices.size(), objectTypeVertices.data()) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}
}

void OpenClRayTracer::writeToInstanceBuffer() {
	if (queue.enqueueWriteBuffer(objectInstanceBuffer, CL_TRUE, 0, sizeof(Instance) * objectInstances.size(), objectInstances.data()) != CL_SUCCESS) {
		std::cout << "Failed to write to buffer" << std::endl;
		exit(1);
	}
}

void OpenClRayTracer::rayTrace(float16 matrix) {
	rayTraceNonBlocking(matrix);// .wait();
	queue.finish();
	draw();
}


void OpenClRayTracer::computeOnCPU()
{
	//TODO make me work, or move me out of here
	//result = cpuRayTrace(width, height, objectTypes, triangles, objectTypeVertices);
}

//Also writes to instanceBuffer
cl::Event OpenClRayTracer::vertexShaderNonBlocking() {
	writeToInstanceBuffer();

	if (vertexShaderKernel.setArg(0, objectTypeBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (vertexShaderKernel.setArg(1, objectInstanceBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}

	if (vertexShaderKernel.setArg(2, objectTypeVertexBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}

	if (vertexShaderKernel.setArg(3, transformedObjectBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (vertexShaderKernel.setArg(4, transformedVertexBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}

	queue.finish();

	
	cl::Event event;

	queue.enqueueNDRangeKernel(vertexShaderKernel, cl::NullRange, cl::NDRange(objectInstances.size()), cl::NullRange, 0, &event);

	
	return event;

}

cl::Event OpenClRayTracer::aabbNonBlocking() {
	
	if (aabbKernel.setArg(0, transformedVertexBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (aabbKernel.setArg(1, transformedObjectBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}

	queue.finish();

	cl::Event event;

	queue.enqueueNDRangeKernel(aabbKernel, cl::NullRange, cl::NDRange(objectInstances.size()), cl::NullRange, 0, &event);

	return event;

}

//Give me a better name
cl::Event OpenClRayTracer::prepRayTraceNonBlocking() {

	vertexShaderNonBlocking().wait();

	return aabbNonBlocking();
}

cl::Event OpenClRayTracer::rayTraceNonBlocking(float16 matrix) {
	
	
	//Make sure OpenGL is done working
	glFinish();


	//Take ownership of OpenGL texture
	if (queue.enqueueAcquireGLObjects(&resultImages, NULL, NULL) != CL_SUCCESS) {
		std::cout << "Failed to acquire result Texture from OpenGL" << std::endl;
		exit(1);
	}
	queue.finish();//Make sure OpenCL has grabbed the texture from GL(probably not needed)


	

	int instanceCount = objectInstances.size();
	if (rayTraceKernel.setArg(0, sizeof(instanceCount), &instanceCount) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (rayTraceKernel.setArg(1, sizeof(float16), &matrix) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (rayTraceKernel.setArg(2, transformedObjectBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (rayTraceKernel.setArg(3, objectTypeIndexBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (rayTraceKernel.setArg(4, transformedVertexBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	if (rayTraceKernel.setArg(5, resultImages[0]) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}
	
	queue.finish();

	cl::Event event;


	int status = queue.enqueueNDRangeKernel(rayTraceKernel, cl::NullRange, cl::NDRange(width, height), cl::NullRange, 0, &event);
	if (status != CL_SUCCESS) {
		std::cout << "Failed to enqueue rayTraceKernel with error: " << status << std::endl;
		system("pause");
		exit(1);
	}

	return event;
}



void OpenClRayTracer::debugCl() {
	int counter;

	counter = 0;

	cl::Buffer countBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(counter), &counter);


	if (debugKernel.setArg(0, countBuffer) != CL_SUCCESS) {
		std::cout << "Failed to set argument" << std::endl;
		exit(1);
	}

	cl::Event event;
	queue.enqueueNDRangeKernel(debugKernel, cl::NullRange, cl::NDRange(1000), cl::NullRange);
	queue.finish();
	queue.enqueueReadBuffer(countBuffer, CL_TRUE, 0, sizeof(counter), &counter);
	int cSize = sizeof(Instance);
	if(counter != cSize)
		std::cout << "Not matching!" << std::endl;
}


void OpenClRayTracer::draw() {

	//Give back ownership of OpenGL texture
	queue.enqueueReleaseGLObjects(&resultImages, NULL, NULL);
	queue.finish();//Make wait for it to be released


	renderer.draw();
}

void OpenClRayTracer::compabilityDraw() {

	queue.finish();
	queue.enqueueReadBuffer(resultPixels, CL_TRUE, 0, sizeof(float4) * width * height, colors.data());


	renderer.writeToScreen(colors);



	renderer.draw();

}

void OpenClRayTracer::initializeAdvancedRender() {
	rayBuffers.reserve(RAY_DEPTH);
	rayTreeBuffers.reserve(RAY_DEPTH);
	hitBuffers.reserve(RAY_DEPTH + 1);

	for (int i = 0; i < RAY_DEPTH; i++) {
		size_t raySize = sizeof(Ray);
		size_t rayBufferSize = sizeof(Ray) * width * height * (1 << (i));
		rayBuffers.push_back(cl::Buffer(context, CL_MEM_READ_WRITE, rayBufferSize));
		rayTreeBuffers.push_back(cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(RayTree) * width * height * (1 << i)));
		hitBuffers.push_back(cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(Hit) * width * height * (1 << i)));
	}

}

void OpenClRayTracer::render(float16 matrix) {

	queue.finish();

	cl::Event prepRaytracingEvent = prepRayTraceNonBlocking();//Is allowed to run in parallel to perspectiveRayGeneratorKernel


	auto startTime = std::chrono::high_resolution_clock::now();

	cl_int rayCount = width * height;
	perspectiveRayGeneratorKernel.setArg(0, matrix);
	perspectiveRayGeneratorKernel.setArg(1, rayBuffers[0]);
	queue.enqueueNDRangeKernel(perspectiveRayGeneratorKernel, cl::NullRange, cl::NDRange(width, height));

	prepRaytracingEvent.wait();
	queue.finish();

	
	std::vector<TimePoint> rayTracerStartTimes;
	rayTracerStartTimes.push_back(std::chrono::high_resolution_clock::now());


	int instanceCount = objectInstances.size();
	rayTraceAdvancedKernel.setArg(0, sizeof(instanceCount), &instanceCount);
	
#ifdef BVH
	int argumentOffset = 2;
	rayTraceAdvancedKernel.setArg(1, objectInstanceBuffer);
	rayTraceAdvancedKernel.setArg(2, objectTypeBuffer);
	rayTraceAdvancedKernel.setArg(3, objectTypeTriangleBvhNodesBuffer);
	rayTraceAdvancedKernel.setArg(4, objectTypeIndexBuffer);
	rayTraceAdvancedKernel.setArg(5, objectTypeVertexBuffer);
	rayTraceAdvancedKernel.setArg(6, rayBuffers[0]);
	rayTraceAdvancedKernel.setArg(7, hitBuffers[0]);
	rayTraceAdvancedKernel.setArg(8, rayTreeBuffers[0]);
#else
	int argumentOffset = 0;
	rayTraceAdvancedKernel.setArg(1, transformedObjectBuffer);
	rayTraceAdvancedKernel.setArg(2, objectTypeIndexBuffer);
	rayTraceAdvancedKernel.setArg(3, transformedVertexBuffer);
	rayTraceAdvancedKernel.setArg(4, rayBuffers[0]);
	rayTraceAdvancedKernel.setArg(5, hitBuffers[0]);
	rayTraceAdvancedKernel.setArg(6, rayTreeBuffers[0]);
#endif
	queue.enqueueNDRangeKernel(rayTraceAdvancedKernel, cl::NullRange, cl::NDRange(width * height));
	queue.finish();




	cl::Buffer rayCountBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &rayCount);
	rayGeneratorKernel.setArg(1, rayCountBuffer);


	std::vector<cl_int> rayCounts(RAY_DEPTH);


	std::vector<TimePoint> rayGeneratorStartTimes;

	int i;
	for (i = 0; i < RAY_DEPTH - 1; i++) {//Continue until maximum ray depth is reached or no more rays left to trace
		if (rayCount > (1 << i) * width * height)
			throw std::exception("Too large rayCount! Probably caused by some bug");			//Probably caused by some syncronization bug

		rayGeneratorStartTimes.push_back(std::chrono::high_resolution_clock::now());


		rayCounts[i] = rayCount;

		cl_int startIndex;
		rayGeneratorKernel.setArg(0, hitBuffers[i]);
		rayGeneratorKernel.setArg(2, rayBuffers[i + 1]);
		rayGeneratorKernel.setArg(3, rayTreeBuffers[i]);
		startIndex = 0;																				//
		queue.enqueueWriteBuffer(rayCountBuffer, CL_TRUE, 0, sizeof(cl_int), &startIndex);			//Reset rayIndex to 0
		queue.enqueueNDRangeKernel(rayGeneratorKernel, cl::NullRange, cl::NDRange(rayCount));
		queue.finish();
		queue.enqueueReadBuffer(rayCountBuffer, CL_TRUE, 0, sizeof(cl_int), &rayCount);

		if (!rayCount) {//No rays left to trace(parents weren't refractive/reflective)
			break;
		}

		rayTracerStartTimes.push_back(std::chrono::high_resolution_clock::now());


		rayTraceAdvancedKernel.setArg(4 + argumentOffset, rayBuffers[i + 1]);
		rayTraceAdvancedKernel.setArg(5 + argumentOffset, hitBuffers[i + 1]);
		rayTraceAdvancedKernel.setArg(6 + argumentOffset, rayTreeBuffers[i + 1]);
		queue.enqueueNDRangeKernel(rayTraceAdvancedKernel, cl::NullRange, cl::NDRange(rayCount));
		queue.finish();


	}
	
	std::vector<TimePoint> treeTraverserStartTimes;

	for (; i > 0; i--) {
		rayCount = rayCounts[i - 1];
		//rayCounts.pop_back();
		
		treeTraverserStartTimes.insert(treeTraverserStartTimes.begin(), std::chrono::high_resolution_clock::now());


		treeTraverserKernel.setArg(0, rayTreeBuffers[i - 1]);//[i - 1]);
		treeTraverserKernel.setArg(1, rayTreeBuffers[i]);
		queue.enqueueNDRangeKernel(treeTraverserKernel, cl::NullRange, cl::NDRange(rayCount));
		queue.finish();

	}

	auto colorToPixelStartTime = std::chrono::high_resolution_clock::now();

	TimePoint drawingStartTime;


	if (useInterop) {
		try {//TODO: Find better solution
			colorToPixelKernel.setArg(0, rayTreeBuffers[0]);
			colorToPixelKernel.setArg(1, resultImages[0]);
			queue.enqueueNDRangeKernel(colorToPixelKernel, cl::NullRange, cl::NDRange(width, height));
			queue.finish();

			drawingStartTime = std::chrono::high_resolution_clock::now();

			draw();
		}
		catch (cl::Error e) {
			useInterop = false;
			resultPixels = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float4) * width * height);
			colors = std::vector<float4>(width * height);
		}
	}
	if (!useInterop) {
		
		colorToColorKernel.setArg(0, rayTreeBuffers[0]);
		colorToColorKernel.setArg(1, resultPixels);
		
		queue.enqueueNDRangeKernel(colorToColorKernel, cl::NullRange, cl::NDRange(width, height));
		
		queue.finish();
		drawingStartTime = std::chrono::high_resolution_clock::now();

		compabilityDraw();
	}

	auto doneTime = std::chrono::high_resolution_clock::now();

	
	
	
	profileAdvancedRender(
		startTime,
		rayTracerStartTimes,
		rayGeneratorStartTimes,
		treeTraverserStartTimes,
		colorToPixelStartTime,
		drawingStartTime,
		doneTime
	);
}

void OpenClRayTracer::profileAdvancedRender(
	TimePoint startTime,
	std::vector<TimePoint> rayTracerStartTimes,
	std::vector<TimePoint> rayGeneratorStartTimes,
	std::vector<TimePoint> treeTraverserStartTimes,
	TimePoint colorToPixelStartTime,
	TimePoint drawingStartTime,
	TimePoint doneTime
) {
	static auto lastTimeOfReport = std::chrono::high_resolution_clock::now();
	auto deltaTime = doneTime - lastTimeOfReport;
	if (deltaTime < std::chrono::seconds(1))
		return;

	lastTimeOfReport += deltaTime;
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "----Total Time: " << durationToMs(doneTime - startTime) << "ms----" << std::endl;
	std::cout << "--------" << std::endl;
	
	std::cout << "PerspectiveRayGenerator: " << durationToMs(rayTracerStartTimes.front() - startTime) << "ms----" << std::endl;
	for (int i = 0; i < std::min(rayTracerStartTimes.size() - 1, rayGeneratorStartTimes.size()); i++) {
		std::cout << "RayGenerator run(" << (i + 1) << "): " << durationToMs(rayTracerStartTimes[i + 1] - rayGeneratorStartTimes[i]) << "ms----" << std::endl;
	}
	if (rayTracerStartTimes.size() - rayGeneratorStartTimes.size() == 1) {
		std::cout << "RayGenerator run(" << (rayGeneratorStartTimes.size()) << "): " << durationToMs(rayTracerStartTimes.back() - rayGeneratorStartTimes.back()) << "ms----" << std::endl;
	}
	std::cout << "--------" << std::endl;
	
	std::cout << "RayTracer run(0): " << durationToMs(rayGeneratorStartTimes.front() - rayTracerStartTimes.front()) << "ms----" << std::endl;
	for (int i = 0; i < rayGeneratorStartTimes.size() - 1; i++) {
		std::cout << "RayTracer run(" << (i + 1) << "): " << durationToMs(rayGeneratorStartTimes[i + 1] - rayTracerStartTimes[i + 1]) << "ms----" << std::endl;
	}
	if (treeTraverserStartTimes.size()) {
		std::cout << "RayTracer run(" << (rayGeneratorStartTimes.size()) << "): " << durationToMs(treeTraverserStartTimes.front() - rayTracerStartTimes.back()) << "ms----" << std::endl;
		std::cout << "--------" << std::endl;
	}

	for (int i = 1; i < treeTraverserStartTimes.size(); i++) {
		std::cout << "TreeTraverse run(" << (i - 1) << "): " << durationToMs(treeTraverserStartTimes[i] - treeTraverserStartTimes[i - 1]) << "ms----" << std::endl;
	}

	if (treeTraverserStartTimes.size()) {
		std::cout << "TreeTraverse run(" << (treeTraverserStartTimes.size()) << "): " << durationToMs(colorToPixelStartTime - treeTraverserStartTimes.back()) << "ms----" << std::endl;
	}
	else {
		std::cout << "RayTracer run(" << (rayTracerStartTimes.size()) << "): " << durationToMs(colorToPixelStartTime - rayTracerStartTimes.back()) << "ms----" << std::endl;
	}
	std::cout << "--------" << std::endl;

	std::cout << "ColorToPixel: " << durationToMs(drawingStartTime - colorToPixelStartTime) << "ms----" << std::endl;
	std::cout << "Drawing: " << durationToMs(doneTime - drawingStartTime) << "ms----" << std::endl;
	for(int i = 0; i < 10; i++)
		std::cout << std::endl;
}

double OpenClRayTracer::durationToMs(Duration duration) {
	return std::chrono::duration<double, std::milli>(duration).count();
}


void OpenClRayTracer::resizeCallback(GLFWwindow* window, int width, int height) {
	this->width = width;
	this->height = height;

#ifdef ADVANCED_RENDERER
	throw "Resize Not implemented!";
#endif // ADVANCED_RENDERER


	renderer.resizeCallback(window, width, height);

	cl_int status;



	resultImages[0] = cl::ImageGL(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, openGlTextureID, &status);
	if (status != CL_SUCCESS) {
		std::cout << "Failed to create OpenCL image from OpenGL texture" << std::endl;
		exit(1);
	}

}