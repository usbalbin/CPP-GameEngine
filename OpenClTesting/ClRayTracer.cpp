#include "stdafx.h"

#include "ClRayTracer.hpp"
#include "OpenClContexts.hpp"
#include "DebugRayTracer.hpp"
#include "Utils.hpp"
#include "BvhTree.hpp"

#include "ClReadBuffer.hpp"

#include "GLFW\glfw3.h"

#include <iostream>
#include <stack>
#include <atomic>
#include <iomanip>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif // _WIN32

ClRayTracer::ClRayTracer() {

}

ClRayTracer::ClRayTracer(int width, int height, bool forceCpu, bool fullScreen) :
	renderer(width, height, fullScreen, "shaders/vertexShader.glsl", "shaders/fragmentShader.glsl",
		[](GLFWwindow* window, int width, int height) {
	ClRayTracer* rayTracer = (ClRayTracer*)glfwGetWindowUserPointer(window);

	rayTracer->resizeCallback(window, width, height);
}) {

	glfwSetWindowUserPointer(renderer.getWindow(), this);

	this->width = width;
	this->height = height;
	this->forceCpu = forceCpu;
	this->resultImages.resize(1);
	this->openGlTextureID = renderer.setupScreen();
	initialize();
}

ClRayTracer::~ClRayTracer()
{
}

void ClRayTracer::initialize() {
	OpenClContexts openClContexts;

	cl::Device device;
	if (!forceCpu) {
		if (useInterop) {
			try {
				openClContexts.initializeInteropGpu();
				device = openClContexts.getGpuDevice(GPU_DEVICE_ID);
				this->context = openClContexts.getGpuContext(GPU_CONTEXT_ID);
			}
			catch (cl::Error e) {
				useInterop = false;
			}
		}
		if (!useInterop) {
			try {
				openClContexts.initializeGpu();
				openClContexts.getGpuDevice(GPU_DEVICE_ID);
				this->context = openClContexts.getGpuContext(GPU_CONTEXT_ID);
			}
			catch (cl::Error e) {
				openClContexts.initializeCpu();
				device = openClContexts.getCpuDevice(0);
				this->context = openClContexts.getCpuContext(0);
			}
		}
	} else {
		openClContexts.initializeCpu();
		device = openClContexts.getCpuDevice(0);
		this->context = openClContexts.getCpuContext(0);
		useInterop = false;
	}

	materialManager = MaterialManager(MAX_TEXTURE_COUNT, context);

	cl::Program::Sources sources;
	std::string debugSource = readFileToString("kernels/debug.cl");
	std::string perspectiveRayGeneratorSource = readFileToString("kernels/newKernels/1_perspectiveRayGenerator.cl");
	std::string rayGeneratorSource = readFileToString("kernels/newKernels/2A_rayGenerator.cl");
	std::string rayTraceAdvancedSource = readFileToString("kernels/newKernels/2B_rayTracer.cl");
	std::string treeTraverserSource = readFileToString("kernels/newKernels/3_treeTraverser.cl");
	std::string colorToPixelSource = readFileToString("kernels/newKernels/4_colorToPixel.cl");


	sources.push_back( "#define TEXTURE_ARGS " + materialManager.genOclTextureArgumentsCode() + "\n" );
	sources.push_back( "#define GET_TEXTURE_COLOR(RES_COLOR,TEX_ID,TEX_POS) " + materialManager.genOclTextureBlendCode() + "\n");
	sources.push_back({ debugSource.c_str(), debugSource.length() });
	sources.push_back({ perspectiveRayGeneratorSource.c_str(), perspectiveRayGeneratorSource.length() });
	sources.push_back({ rayTraceAdvancedSource.c_str(), rayTraceAdvancedSource.length() });
	sources.push_back({ rayGeneratorSource.c_str(), rayGeneratorSource.length() });
	sources.push_back({ treeTraverserSource.c_str(), treeTraverserSource.length() });
	sources.push_back({ colorToPixelSource.c_str(), colorToPixelSource.length() });


	writeSourcesToFile(sources, "kernels/output/allKernels.cl");
	cl::Program program(context, sources);


	std::cout << "---------------- Compilation status ----------------" << std::endl;
	std::string compileMessage;
	char programPathBuffer[256];
	getcwd(programPathBuffer, 256);
	std::string programPath = programPathBuffer;
	

	std::string extraOptions = ""/*= "-cl-std=CL1.1 "*//*-s \"" + programPath + "kernels/newKernels/3_treeTraverser.cl\" "*/;
	extraOptions += "-cl-unsafe-math-optimizations -cl-fast-relaxed-math";

	std::string defines = "";
	defines += " -D BVH ";


	std::string compilerFlags = /*-O0 -g*/" -I " + programPath + " " + extraOptions + defines;
	std::cout << compilerFlags << std::endl;
	try {
		std::cout << "Build started..." << std::endl;
		program.build({ device }, compilerFlags.c_str());
	}
	catch (cl::Error e) {
		glfwDestroyWindow(renderer.getWindow());
		std::cout << "Prepping error message..." << std::endl;
		compileMessage = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << "Failed to compile with status " << e.err() << ": " << compileMessage << std::endl;
		system("pause");
		exit(1);
	}

	compileMessage = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	std::cout << compileMessage << std::endl;

	queue = cl::CommandQueue(context, device, NULL);


	debugKernel = cl::Kernel(program, "debug");
	perspectiveRayGeneratorKernel = cl::Kernel(program, "perspectiveRayGenerator");
	rayTraceAdvancedKernel = cl::Kernel(program, "rayTraceAdvanced");
	rayGeneratorKernel = cl::Kernel(program, "rayGenerator");
	treeTraverserKernel = cl::Kernel(program, "treeTraverser");
	colorToPixelKernel = cl::Kernel(program, "colorToPixel");
	colorToColorKernel = cl::Kernel(program, "colorToColor");


	objectTypes.setup(queue, context);
	objectTypeTriangleBvhNodes.setup(queue, context);
	objectTypeIndices.setup(queue, context);
	objectTypeVertices.setup(queue, context);
	objectInstances.setup(queue, context);


	if (useInterop) {
		resultImages[0] = cl::ImageGL(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, openGlTextureID);
	}
	else {
		resultPixels = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float4) * width * height);
		colors = std::vector<float4>(width * height);
	}

}


void ClRayTracer::clear() {
	this->objectInstances.clear();
	this->transformedVertexCount = 0;
}

void ClRayTracer::push_back(Instance instance) {
	instance.startVertex = this->transformedVertexCount;

	Object objectType = this->objectTypes[instance.meshType];

	this->objectInstances.push_back(instance);
	this->transformedVertexCount += objectType.numVertices;
}

void ClRayTracer::push_back(MultiInstance& multiInstance)
{
	for (auto& instance : multiInstance.instances)
		push_back(instance);
}


InstanceBuilder ClRayTracer::push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices) {
	Object objectType;
	BvhTree bvhTree(objectTypeIndices, objectTypeVertices);

	objectType.bvhRootNodeIndex = this->objectTypeTriangleBvhNodes.size();
	objectType.bvhTreeSize = bvhTree.getCurNodeIndex();
	bvhTree.appendNodesToVector(objectTypeTriangleBvhNodes.getHostData());

	objectType.startTriangle = this->objectTypeIndices.size();
	objectType.startVertex = this->objectTypeVertices.size();
	objectType.numTriangles = objectTypeIndices.size();
	objectType.numVertices = objectTypeVertices.size();

	this->objectTypes.push_back(objectType);
	this->objectTypeIndices.insert_back(objectTypeIndices);
	this->objectTypeVertices.insert_back(objectTypeVertices);

	const int meshType = objectTypes.size() - 1;
	InstanceBuilder instanceBuilder(objectType, meshType);

	return instanceBuilder;
}

InstanceBuilder ClRayTracer::push_backObjectType(std::vector<TriangleIndices>& objectTypeIndices, std::vector<Vertex>& objectTypeVertices, const InstanceBuilder & builderWithCommonVertices)
{
	Object objectType;
	BvhTree bvhTree(objectTypeIndices, objectTypeVertices);

	objectType.bvhRootNodeIndex = this->objectTypeTriangleBvhNodes.size();
	objectType.bvhTreeSize = bvhTree.getCurNodeIndex();
	bvhTree.appendNodesToVector(objectTypeTriangleBvhNodes.getHostData());

	objectType.startTriangle = this->objectTypeIndices.size();
	objectType.startVertex = builderWithCommonVertices.startVertex;
	objectType.numTriangles = objectTypeIndices.size();
	objectType.numVertices = builderWithCommonVertices.numVertices;

	this->objectTypes.push_back(objectType);
	this->objectTypeIndices.insert_back(objectTypeIndices);

	const int meshType = objectTypes.size() - 1;
	InstanceBuilder instanceBuilder(objectType, meshType);

	return instanceBuilder;
}

//depricated
Instance ClRayTracer::makeInstanceOld(std::string meshPath, float16 initialTransform) {
	if (builders.find(meshPath) != builders.end())
		return Instance(initialTransform, glm::inverse(initialTransform), builders[meshPath]);

	float reflection = 0;
	float refraction = 0;
	std::vector<Vertex> vertices;
	std::vector<TriangleIndices> indices;

	std::string texturePath;
	readObjFile(vertices, indices, texturePath, meshPath);
	builders[meshPath] = push_backObjectType(indices, vertices);
	builders[meshPath].texId = texturePath.length() ? materialManager.getTextureId(texturePath) : -1;
	
	return Instance(initialTransform, glm::inverse(initialTransform), builders[meshPath]);
}

MultiInstance ClRayTracer::makeInstance(std::string meshPath, float16 initialTransform)
{
	if (multiBuilders.find(meshPath) != multiBuilders.end())
		return MultiInstance(initialTransform, multiBuilders[meshPath]);

	float reflection = 0;
	float refraction = 0;
	std::vector<Vertex> vertices;
	std::unordered_map<std::string, std::vector<TriangleIndices>> indices;
	MultiInstanceBuilder multiBuilder;
	
	readObjMulti(vertices, indices, meshPath);
	InstanceBuilder firstBuilder;
	{//First builder will have all vertices
		auto& firstP = *indices.begin();
		auto& indexList = firstP.second;
		const std::string& materialName = firstP.first;
		const Material& material = materialManager.getMaterial(materialName);
		const std::string& texturePath = material.texturePath;

		std::string builderKey = meshPath + ':' + materialName;

		firstBuilder = push_backObjectType(indexList, vertices);
		firstBuilder.texId = materialManager.getTextureId(texturePath);
		multiBuilder.instanceBuilders.push_back(firstBuilder);

		indices.erase(firstP.first);
	}

	for (auto& p : indices) {//The rest of the builders will use the first ones vertices
		auto& indexList = p.second;
		const std::string& materialName = p.first;
		const Material& material = materialManager.getMaterial(materialName);
		const std::string& texturePath = material.texturePath;

		std::string builderKey = meshPath + ':' + materialName;


		auto builder = push_backObjectType(indexList, vertices, firstBuilder);
		builder.texId = materialManager.getTextureId(texturePath);
		multiBuilder.instanceBuilders.push_back(builder);
	}
	multiBuilders[meshPath] = multiBuilder;
	return MultiInstance(initialTransform, multiBuilder);
}

void ClRayTracer::readObjMulti(std::vector<Vertex>& verticesOut, std::unordered_map<std::string, std::vector<TriangleIndices>>& indicesOut, std::string & filePath, float reflection, float refraction) {
	std::ifstream objFile;
	objFile.open(filePath);
	if (!objFile)
		throw "Failed to open file " + filePath;

	

	std::string line;
	std::string directory = filePath.substr(0, filePath.find_last_of("/\\"));

	std::vector<float3> positions;
	std::vector<float2> texturePositions;
	std::vector<float3> normals;
	std::unordered_map<std::string, std::vector<Face>> faces;
	std::string materialName = filePath;

	std::map<Vertex, int> vertexMap;

	//Key is materialName, value is libFile
	std::map<std::string, std::string> nameToMtlLib;

	while (getline(objFile, line)) {
		line = line.substr(0, line.find("#"));

		if (line.find("usemtl ") != line.npos)
			materialName = nameToMtlLib[line.substr(7)] + ':' + line.substr(7);

		else if (line.find("mtllib ") != line.npos) {
			auto materialNames = materialManager.readMtlFileMulti(directory + "/" + line.substr(7));
			for (auto name : materialNames)
				nameToMtlLib[name] = directory + "/" + line.substr(7);
		}

		else if (line.find("v ") != line.npos)
			positions.push_back(parseFloat3(line.substr(2)));

		else if (line.find("vn ") != line.npos)
			normals.push_back(parseFloat3(line.substr(3)));

		else if (line.find("vt ") != line.npos)
			texturePositions.push_back(parseFloat2(line.substr(3)));


		else if (line.find("f ") != line.npos)
			faces[materialName].push_back(parseFace(line.substr(2)));

	}

	for (auto& p : faces) {
		auto& material = p.first;
		auto& faceList = p.second;

		for (Face& face : faceList) {
			TriangleIndices triangleIndices;

			addVertex(std::get<0>(face), &triangleIndices.a, vertexMap, verticesOut, positions, texturePositions, normals, reflection, refraction);
			addVertex(std::get<1>(face), &triangleIndices.b, vertexMap, verticesOut, positions, texturePositions, normals, reflection, refraction);
			addVertex(std::get<2>(face), &triangleIndices.c, vertexMap, verticesOut, positions, texturePositions, normals, reflection, refraction);

			indicesOut[material].push_back(triangleIndices);
		}
	}

	if (indicesOut[filePath].size() < 1)//Remove default material if not being used
		indicesOut.erase(filePath);
}

void ClRayTracer::getMeshData(const Instance & instance, std::vector<TriangleIndices>& indices, std::vector<Vertex>& vertices) {
	const Object object = objectTypes[instance.meshType];
	
	std::copy(&objectTypeIndices[object.startTriangle], &objectTypeIndices[object.startTriangle + object.numTriangles], indices.begin());
	
	std::copy(&objectTypeVertices[object.startTriangle], &objectTypeVertices[object.startVertex + object.numVertices], vertices.begin());
}

void ClRayTracer::getMeshVertices(const Instance & instance, std::vector<Vertex>& verticesOut)
{
	const Object object = objectTypes[instance.meshType];

	std::copy(&objectTypeVertices[object.startTriangle], &objectTypeVertices[object.startVertex + object.numVertices], verticesOut.begin());
}

void ClRayTracer::getMeshPointsJoinedMulti(const MultiInstance & instance, float*& const vertices, int& vertexCount, int& stride) {
	getMeshPoints(instance.instances[0], vertices, vertexCount, stride);
}
void ClRayTracer::getMeshPoints(const Instance & instance, float*& const vertices, int& vertexCount, int& stride) {
	const Object object = objectTypes[instance.meshType];
	vertices = &(objectTypeVertices[object.startVertex].position.x);
	vertexCount = object.numVertices;
	stride = sizeof(Vertex);
}


void ClRayTracer::debugCl() {
	int counter = 0;
	cl::Buffer countBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(counter), &counter);
	debugKernel.setArg(0, countBuffer);

	queue.enqueueNDRangeKernel(debugKernel, cl::NullRange, cl::NDRange(1000), cl::NullRange);
	queue.finish();
	queue.enqueueReadBuffer(countBuffer, CL_TRUE, 0, sizeof(counter), &counter);
}


void ClRayTracer::draw() {
	//Give back ownership of OpenGL texture
	queue.enqueueReleaseGLObjects(&resultImages, NULL, NULL);
	queue.finish();//Make wait for it to be released

	renderer.draw();
}

void ClRayTracer::compabilityDraw() {

	queue.finish();
	queue.enqueueReadBuffer(resultPixels, CL_TRUE, 0, sizeof(float4) * width * height, colors.data());

	renderer.writeToScreen(colors);
	renderer.draw();
}

void ClRayTracer::initializeAdvancedRender() {
	rayBuffers.reserve(RAY_DEPTH);
	rayTreeBuffers.reserve(RAY_DEPTH);
	hitBuffers.reserve(RAY_DEPTH + 1);

	for (int i = 0; i < RAY_DEPTH; i++) {
		size_t raySize = sizeof(Ray);
		size_t rayBufferSize = sizeof(Ray) * width * height * (1 << i);
		rayBuffers.emplace_back(context, CL_MEM_READ_WRITE, rayBufferSize);
		rayTreeBuffers.emplace_back(context, CL_MEM_READ_WRITE, sizeof(RayTree) * width * height * (1 << i));
		hitBuffers.emplace_back(context, CL_MEM_READ_WRITE, sizeof(Hit) * width * height * (1 << i));
	}

}

void ClRayTracer::render(float16 matrix) {
	queue.finish();


	auto startTime = std::chrono::high_resolution_clock::now();

	cl_int rayCount = width * height;
	perspectiveRayGeneratorKernel.setArg(0, matrix);
	perspectiveRayGeneratorKernel.setArg(1, rayBuffers[0]);
	queue.enqueueNDRangeKernel(perspectiveRayGeneratorKernel, cl::NullRange, cl::NDRange(width, height));
	queue.finish();

	std::vector<RtTimePoint> rayTracerStartTimes;
	rayTracerStartTimes.push_back(std::chrono::high_resolution_clock::now());


	int instanceCount = objectInstances.size();
	rayTraceAdvancedKernel.setArg(0, sizeof(instanceCount), &instanceCount);
	rayTraceAdvancedKernel.setArg(1, objectInstances());
	rayTraceAdvancedKernel.setArg(2, objectTypes());
	rayTraceAdvancedKernel.setArg(3, objectTypeTriangleBvhNodes());
	rayTraceAdvancedKernel.setArg(4, objectTypeIndices());
	rayTraceAdvancedKernel.setArg(5, objectTypeVertices());
	rayTraceAdvancedKernel.setArg(6, rayBuffers[0]);
	rayTraceAdvancedKernel.setArg(7, hitBuffers[0]);
	rayTraceAdvancedKernel.setArg(8, rayTreeBuffers[0]);

	materialManager.setTextureArguments(9, rayTraceAdvancedKernel);

	queue.enqueueNDRangeKernel(rayTraceAdvancedKernel, cl::NullRange, cl::NDRange(width * height));
	queue.finish();


	cl::Buffer rayCountBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &rayCount);
	rayGeneratorKernel.setArg(1, rayCountBuffer);

	std::vector<cl_int> rayCounts(RAY_DEPTH);
	std::vector<RtTimePoint> rayGeneratorStartTimes;

	int i;
	for (i = 0; i < RAY_DEPTH - 1; i++) {//Continue until maximum ray depth is reached or no more rays left to trace
		if (rayCount >(1 << i) * width * height)
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

		rayTraceAdvancedKernel.setArg(6, rayBuffers[i + 1]);
		rayTraceAdvancedKernel.setArg(7, hitBuffers[i + 1]);
		rayTraceAdvancedKernel.setArg(8, rayTreeBuffers[i + 1]);
		queue.enqueueNDRangeKernel(rayTraceAdvancedKernel, cl::NullRange, cl::NDRange(rayCount));
		queue.finish();
	}

	std::vector<RtTimePoint> treeTraverserStartTimes;

	for (; i > 0; i--) {
		rayCount = rayCounts[i - 1];

		treeTraverserStartTimes.insert(treeTraverserStartTimes.begin(), std::chrono::high_resolution_clock::now());


		treeTraverserKernel.setArg(0, rayTreeBuffers[i - 1]);//[i - 1]);
		treeTraverserKernel.setArg(1, rayTreeBuffers[i]);
		queue.enqueueNDRangeKernel(treeTraverserKernel, cl::NullRange, cl::NDRange(rayCount));
		queue.finish();

	}

	auto colorToPixelStartTime = std::chrono::high_resolution_clock::now();
	RtTimePoint drawingStartTime;

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

void ClRayTracer::profileAdvancedRender(
	RtTimePoint startTime,
	std::vector<RtTimePoint> rayTracerStartTimes,
	std::vector<RtTimePoint> rayGeneratorStartTimes,
	std::vector<RtTimePoint> treeTraverserStartTimes,
	RtTimePoint colorToPixelStartTime,
	RtTimePoint drawingStartTime,
	RtTimePoint doneTime
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
	for (int i = 0; i < 10; i++)
		std::cout << std::endl;
}

double ClRayTracer::durationToMs(RtDuration duration) {
	return std::chrono::duration<double, std::milli>(duration).count();
}

void ClRayTracer::resizeCallback(GLFWwindow* window, int width, int height) {
	this->width = width;
	this->height = height;

	renderer.resizeCallback(window, width, height);

	rayBuffers.clear();
	rayTreeBuffers.clear();
	hitBuffers.clear();
	initializeAdvancedRender();

	if(useInterop)
		resultImages[0] = cl::ImageGL(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, openGlTextureID);
	else {
		resultPixels = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float4) * width * height);
		colors = std::vector<float4>(width * height);
	}
}