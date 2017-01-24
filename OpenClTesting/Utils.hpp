#pragma once
#define NOMINMAX
#include "Containers.hpp"
#include <map>
#include <string>
#include <CL/cl2.hpp>

typedef std::tuple<int, int, int> FaceElement;
typedef std::tuple<FaceElement, FaceElement, FaceElement> Face;
typedef unsigned char ubyte;
struct ubyte3 {
	ubyte3() {}
	ubyte3(ubyte r, ubyte g, ubyte b) : r(r), g(g), b(b) {}
	ubyte r, g, b;
};

struct ubyte4 {
	ubyte4() {}
	ubyte4(ubyte r, ubyte g, ubyte b, ubyte a) : r(r), g(g), b(b), a(a) {}
	ubyte r, g, b, a;
};

//inline float3 max(float3 l, float3 r);
//inline float3 min(float3 l, float3 r);
std::string readFileToString(std::string filePath);

void writeStringToFile(std::string text, std::string path);
void writeSourcesToFile(cl::Program::Sources sources, std::string path);

void setColor(std::vector<Vertex>& vertices, float4 color);
void calculateNormals(std::vector<Vertex>& vertices, const std::vector<TriangleIndices>& indices);

void readObjFile(std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices, std::string & filePath, float reflection = 0, float refraction = 0);

std::vector<ubyte3> readBmpPixels(std::string& filePath, int* widthOut, int* heightOut);

std::vector<ubyte4> readBmpPixels4(std::string& filePath, int* widthOut, int* heightOut);

void pixelsToMesh(int width, int length, std::vector<ubyte3> colors, std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices);

void addVertex(FaceElement facePart, int * indexOut, std::map<Vertex, int>& vertexMap, std::vector<Vertex>& vertices, std::vector<float3>& positions, std::vector<float3>& normals, float reflection, float refraction);

Vertex facePartToVertex(FaceElement facePart, std::vector<float3>& positions,/* std::vector<float3>& texturePositions,*/ std::vector<float3>& normals);

Face parseFace(std::string line);
FaceElement parseFacePart(std::string line);
float3 parseFloat3(std::string str);


void splitMesh(std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices, std::vector<Vertex>& verticesOut, std::vector<TriangleIndices>& indicesOut);

template<typename T>
T mapToRange(T value, T inLow, T inHigh, T outLow, T outHigh) {
	T deltaIn = inHigh - inLow;
	T deltaOut = outHigh - outLow;

	return (value - inLow) * (deltaOut) / (deltaIn) + outLow;
}