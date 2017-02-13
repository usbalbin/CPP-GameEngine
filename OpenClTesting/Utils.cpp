
#include "stdafx.h"
#include "Utils.hpp"
#include "glm\detail\func_geometric.hpp"
#include <algorithm>


#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>

/*inline float3 max(float3 l, float3 r) {
	return float3(
		std::max(l.x, r.x),
		std::max(l.y, r.y),
		std::max(l.z, r.z)
	);
}

inline float3 min(float3 l, float3 r) {
	return float3(
		std::min(l.x, r.x),
		std::min(l.y, r.y),
		std::min(l.z, r.z)
	);
}*/



std::string readFileToString(std::string filePath) {
	
	std::string fileContents;
	std::ifstream fileStream = std::ifstream(filePath, std::ifstream::in);
	if (!fileStream.is_open()) {
		std::cout << "Failed to open file: \n" << filePath << std::endl;
		system("pause");
		exit(3);
	}

	std::string line = "";
	while (getline(fileStream, line)) {
		fileContents += "\n" + line;
	}
	fileStream.close();


	return fileContents;
	
}

void writeStringToFile(std::string text, std::string path) {
	std::ofstream file;
	file.open(path);
	file << text;
	file.close();
}

void writeSourcesToFile(cl::Program::Sources sources, std::string path) {
	std::ofstream file;
	file.open(path);
	for (auto source : sources) {
		file << source;
	}
	
	file.close();
}

void setColor(std::vector<Vertex>& vertices, float4 color) {
	for (auto& vertex : vertices) {
		vertex.color = color;
	}
}

void calculateNormals(std::vector<Vertex>& vertices, const std::vector<TriangleIndices>& indices) {
	for (auto& triangle : indices) {
		Vertex& a = vertices[triangle.a];
		Vertex& b = vertices[triangle.b];
		Vertex& c = vertices[triangle.c];

		float3 normal = glm::cross(
			a.position - b.position,
			a.position - c.position
		);

		a.normal += normal;
		b.normal += normal;
		c.normal += normal;
	}

	for (auto& vertex : vertices) {
		vertex.normal = glm::normalize(vertex.normal);
	}
}

std::vector<ubyte3> readBmpPixels(std::string& filePath, int* widthOut, int* heightOut) {
	std::ifstream bmpFile(filePath, std::ios_base::binary);
	if (!bmpFile)
		throw filePath + " not found!";
	

	unsigned char header[54];
	bmpFile.read((char*)header, 54);

	int width = *widthOut   = *(int*)&header[18];
	int height = *heightOut = *(int*)&header[22];

	std::vector<ubyte3> pixels(width * height);
	/*while (bmpFile) {
		ubyte3 pixel;
		bmpFile >> pixel.b;
		bmpFile >> pixel.g;
		bmpFile >> pixel.r;
		pixels.push_back(pixel);
	}*/
	bmpFile.read((char*)pixels.data(), sizeof(ubyte3) * pixels.size());

	for (auto& pixel : pixels)//pixel are stored (B,G,R) therefore B and R are swapped
		std::swap(pixel.r, pixel.b);

	return pixels;
}

std::vector<ubyte4> readBmpPixels4(const std::string& filePath, int* widthOut, int* heightOut) {
	std::ifstream bmpFile(filePath, std::ios_base::binary);
	if (!bmpFile)
		throw filePath + " not found!";


	unsigned char header[54];
	bmpFile.read((char*)header, 54);

	int width = *widthOut = *(int*)&header[18];
	int height = *heightOut = *(int*)&header[22];

	std::vector<ubyte4> pixels(width * height);
	for (int i = 0; i < width * height; i++) {
		ubyte4 pixel;
		bmpFile.get((char&)pixel.b);
		bmpFile.get((char&)pixel.g);
		bmpFile.get((char&)pixel.r);
		pixel.a = 255;
		pixels[i] = pixel;
	}

	return pixels;
}

void pixelsToMesh(int width, int length, std::vector<ubyte3> colors, std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices) {
	float textureScale = 10;

	float highest = colors[0].r;//Use red value as height
	float lowest = colors[0].r;

	float reflectFactor = 0.0f;
	glm::vec4 vertexColor(0.5f, 0.5f, 0.5f, 0);


	for (int i = 1; i < colors.size(); i++) {//Find max and min height
		highest = std::max(highest, (float)colors[i].r);
		lowest = std::min(lowest, (float)colors[i].r);
	}

	//Vertices
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < width; j++) {
			float z = mapToRange((float)i, 0.0f, length - 1.0f, -length / 2.0f, +length / 2.0f);
			float x = mapToRange((float)j, 0.0f, width - 1.0f, -width / 2.0f, +width / 2.0f);
		
			auto& color = colors[i * width + j];
			vertices.emplace_back(
				glm::vec3(x, mapToRange((float)color.r, lowest, highest, -1.0f, +1.0f), z),
				glm::vec3(0.0f),
				glm::vec2(x, z) * (1.0f / textureScale)
			);
		}
	}


	//Indices
	for (int z = 0; z < length - 1; z++) {
		for (int x = 0; x < width - 1; x++) {
			int bottomLeft = (x + 0) + (z + 0) * width;
			int bottomRight = (x + 1) + (z + 0) * width;
			int topLeft = (x + 0) + (z + 1) * width;
			int topRight = (x + 1) + (z + 1) * width;

			indices.emplace_back(//Bottom left triangle
				topLeft, bottomRight, bottomLeft
			);

			indices.emplace_back(//Top right triangle
				topLeft, topRight, bottomRight
			);
		}
	}

	calculateNormals(vertices, indices);
}

void readMtlFile(std::string& filePath, std::string& texturePathOut) {
	std::ifstream mtlFile(filePath);
	
	std::string directory = filePath.substr(0, filePath.find_last_of("/\\"));

	std::string line;
	while (getline(mtlFile, line)) {
		if (line.find("map_Kd ") != line.npos)
			texturePathOut = directory + "/" + line.substr(7);
	}
}

void readObjFile(std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices, std::string& texturePathOut, std::string& filePath, glm::vec4& color, float reflection, float refraction) {
	std::ifstream objFile;
	objFile.open(filePath);
	if (!objFile)
		throw "Failed to open file " + filePath;

	texturePathOut = "";

	std::string line;
	std::string directory = filePath.substr(0, filePath.find_last_of("/\\"));

	std::vector<float3> positions;
	std::vector<float2> texturePositions;
	std::vector<float3> normals;
	std::vector<Face> faces;

	std::unordered_map<Vertex, int, VertexHasher> vertexMap;

	while (getline(objFile, line)) {
		line = line.substr(0, line.find("#"));

		if (line.find("mtllib ") != line.npos)
			readMtlFile(directory + "/" + line.substr(7), texturePathOut);

		else if (line.find("v ") != line.npos)
			positions.push_back(parseFloat3(line.substr(2)));
		
		else if (line.find("vn ") != line.npos)
			normals.push_back(parseFloat3(line.substr(3)));

		else if (line.find("vt ") != line.npos)
			texturePositions.push_back(parseFloat2(line.substr(3)));


		else if (line.find("f ") != line.npos) {
			faces.push_back(parseFace(line.substr(2)));
		}
	}

	for (Face& face : faces) {

		TriangleIndices triangleIndices;

		addVertex(std::get<0>(face), &triangleIndices.a, vertexMap, vertices, positions, texturePositions, normals, color, reflection, refraction);

		addVertex(std::get<1>(face), &triangleIndices.b, vertexMap, vertices, positions, texturePositions, normals, color, reflection, refraction);

		addVertex(std::get<2>(face), &triangleIndices.c, vertexMap, vertices, positions, texturePositions, normals, color, reflection, refraction);

		indices.push_back(triangleIndices);
	}
}

void readObjPoints(std::vector<glm::vec3> pointsOut, std::string& filePath) {
	std::ifstream objFile;
	objFile.open(filePath);
	if (!objFile)
		throw "Failed to open file " + filePath;

	std::string line;
	while (getline(objFile, line)) {
		line = line.substr(0, line.find("#"));
		if (line.find("v ") != line.npos)
			pointsOut.push_back(parseFloat3(line.substr(2)));
	}
}

void addVertex(FaceElement facePart, int* indexOut, std::unordered_map<Vertex, int, VertexHasher>& vertexMap, std::vector<Vertex>& vertices, std::vector<float3>& positions, std::vector<float2>& texturePositions, std::vector<float3>& normals, glm::vec4& color, float reflection, float refraction) {
	Vertex vertex = facePartToVertex(facePart, positions, texturePositions, normals);
	vertex.color = color;
	vertex.reflectFactor = reflection;
	vertex.refractFactor = refraction;

	if (vertexMap.find(vertex) == vertexMap.end()) {
		*indexOut = vertices.size();
		vertices.push_back(vertex);
		vertexMap[vertex] = *indexOut;
	}
	else
		*indexOut = vertexMap[vertex];
}

Vertex facePartToVertex(FaceElement facePart, std::vector<float3>& positions, std::vector<float2>& texturePositions, std::vector<float3>& normals) {
	Vertex vertex;
	vertex.position = positions[std::get<0>(facePart)];
	vertex.color = float4(0.2f);

	if (std::get<1>(facePart) != -1)
		vertex.uv = texturePositions[std::get<1>(facePart)];
	else
		vertex.uv = glm::vec2(vertex.position.x, vertex.position.z);

	if (std::get<2>(facePart) != -1)
		vertex.normal = normals[std::get<2>(facePart)];
	else
		vertex.normal = float3(0.0f, 0.0f, 0.0f);
	return vertex;
}

Face parseFace(std::string line) {
	Face result;

	size_t start = 0, end;
		
	end = line.find(" ");
	std::string part = line.substr(start, end);
	std::get<0>(result) = parseFacePart(part);
	start = std::min(end + 1, line.npos);
	
	end = line.find(" ", start);
	part = line.substr(start, end);
	std::get<1>(result) = parseFacePart(part);
	start = std::min(end + 1, line.npos);

	
	part = line.substr(start);
	std::get<2>(result) = parseFacePart(part);


	return result;
}

FaceElement parseFacePart(std::string line) {
	FaceElement result;

	int slashCount = std::count(line.begin(), line.end(), '/') + 1;
	int start = 0, end;

	//Position
	end = line.find("/");
	std::string part = line.substr(start, end - start);
	std::get<0>(result) = std::stoi(part) - 1;
	start = end + 1;

	//Texture position
	if (slashCount > 1 && line.find("//") == line.npos) {
		end = line.find("/", start);
		part = line.substr(start, end - start);
		std::get<1>(result) = std::stoi(part) - 1;
		start = end + 1;
	} else std::get<1>(result) = -1;

	//Normal
	if (slashCount > 2) {
		if (line.find("//") != line.npos)
			start++;
		part = line.substr(start);
		std::get<2>(result) = std::stoi(part) - 1;
	} else std::get<2>(result) = -1;
	
	return result;
}

float3 parseFloat3(std::string str) {
	float3 result;
	size_t pos;

	for (int i = 0; i < 3; i++) {
		result[i] = std::stof(str, &pos);
		str = str.substr(pos);
	}
	return result;
}

float2 parseFloat2(std::string str) {
	float2 result;
	size_t pos;

	for (int i = 0; i < 2; i++) {
		result[i] = std::stof(str, &pos);
		str = str.substr(pos);
	}
	return result;
}

void splitMesh(std::vector<Vertex>& vertices, std::vector<TriangleIndices>& indices, std::vector<Vertex>& verticesOut, std::vector<TriangleIndices>& indicesOut)
{
	std::vector<Vertex> tempVertices;
	std::vector<TriangleIndices> tempIndices;

	AABB aabb(vertices);
	float3 delta = aabb.max - aabb.min;

	int splitDimension = 0;
	float widestDimension = delta.x;
	if (delta.y > widestDimension) {
		widestDimension = delta.y;
		splitDimension = 1;
	}
	if (delta.z > widestDimension) {
		widestDimension = delta.z;
		splitDimension = 2;
	}

	auto sortingFunction = [&](TriangleIndices& thisTriangleIndex, TriangleIndices& otherTriangleIndex) {
		float3 thisCenter = AABB(std::vector<float3>{
			vertices[thisTriangleIndex.a].position,
			vertices[thisTriangleIndex.b].position,
			vertices[thisTriangleIndex.c].position
		}).center();

		float3 otherCenter = AABB(std::vector<float3>{
			vertices[otherTriangleIndex.a].position,
			vertices[otherTriangleIndex.b].position,
			vertices[otherTriangleIndex.c].position
		}).center();

		return thisCenter[splitDimension] < otherCenter[splitDimension];
	};

	std::sort(indices.begin(), indices.end(), sortingFunction);

	std::unordered_map<int, int> dictionary;
	for (int i = 0; i < indices.size() / 2; i++) {
		TriangleIndices triIndex = indices[i];
		TriangleIndices translatedTriIndex;
		for (int j = 0; j < 3; j++) {
			if (dictionary.find(triIndex[j]) != dictionary.end()) {
				translatedTriIndex[j] = dictionary[triIndex[j]];
			}
			else {
				dictionary[triIndex[j]] = translatedTriIndex[j] = tempVertices.size();
				tempVertices.push_back(vertices[triIndex[j]]);
			}
		}
		tempIndices.push_back(translatedTriIndex);
	}
	dictionary.clear();
	for (int i = indices.size() / 2; i < indices.size(); i++) {
		TriangleIndices triIndex = indices[i];
		TriangleIndices translatedTriIndex;
		for (int j = 0; j < 3; j++) {
			if (dictionary.find(triIndex[j]) != dictionary.end()) {
				translatedTriIndex[j] = dictionary[triIndex[j]];
			}
			else {
				dictionary[triIndex[j]] = translatedTriIndex[j] = verticesOut.size();
				verticesOut.push_back(vertices[triIndex[j]]);
			}
		}
		indicesOut.push_back(translatedTriIndex);
	}

	vertices = tempVertices;
	indices = tempIndices;
}