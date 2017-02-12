
#include "stdafx.h"
#include "MaterialManager.hpp"

#include "Utils.hpp"
#include <fstream>

MaterialManager::MaterialManager(int maxTextureCount, cl::Context& context) : maxTextureCount(maxTextureCount), context(context)
{
	currentTextureCount = 0;
	textureBuffers.resize(maxTextureCount);

	ubyte4 dummyPixels[] = {//Checkers texture
		ubyte4(0x00, 0x00, 0x00, 0xFF), ubyte4(0xFF, 0xFF, 0xFF, 0xFF),
		ubyte4(0xFF, 0xFF, 0xFF, 0xFF), ubyte4(0x00, 0x00, 0x00, 0xFF)
	};
	
	for (int i = 0; i < maxTextureCount; i++)
		textureBuffers[i] = cl::Image2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), 2, 2, 0, dummyPixels);
}

std::string MaterialManager::genOclTextureArgumentsCode()
{
	std::string res;

	if(maxTextureCount)
		res += " read_only image2d_t texture0";

	for (int i = 1; i < maxTextureCount; i++) {
		res += ", read_only image2d_t texture" + std::to_string(i);
	}
	return res;
}

std::string MaterialManager::genOclTextureBlendCode()
{
	std::string res =
		"switch(TEX_ID){";
		for (int i = 0; i < maxTextureCount; i++) {
			res += "case " + std::to_string(i) + ":";
			res += "	RES_COLOR = read_imagef(texture" + std::to_string(i) + ", sampler, TEX_POS).xyzw;";
			res += "	break;";
		}
	res += "}";
	return res;
}

void MaterialManager::setTextureArguments(int argCounter, cl::Kernel & kernel)
{
	for (auto& textureBuffer : textureBuffers)
		kernel.setArg(argCounter++, textureBuffer);
}

size_t MaterialManager::getTextureId(const std::string & path) {
	if (path == "")
		return -1;

	if (textureIds.find(path) != textureIds.end())
		return textureIds[path];

	if (currentTextureCount >= textureBuffers.size())
		throw "Texture limit reached!";

	int width, height;
	std::vector<ubyte4> pixels = readBmpPixels4(path, &width, &height);

	textureIds[path] = currentTextureCount;
	textureBuffers[currentTextureCount]= cl::Image2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, cl::ImageFormat(CL_RGBA, CL_UNORM_INT8), width, height, 0, pixels.data());
	return currentTextureCount++;
}

std::vector<std::string> MaterialManager::readMtlFileMulti(std::string& mtlFilePath) {
	if (loadeMtlFiles.find(mtlFilePath) != loadeMtlFiles.end())
		return std::vector<std::string>();													//Already loaded
	
	loadeMtlFiles.insert(mtlFilePath);
	
	std::vector<std::string> res;
	std::ifstream mtlFile(mtlFilePath);

	std::string directory = mtlFilePath.substr(0, mtlFilePath.find_last_of("/\\"));
	std::string materialName = mtlFilePath;

	std::string line;
	while (getline(mtlFile, line)) {
		if (line.find("newmtl ") != line.npos) {
			materialName = mtlFilePath + ':' + line.substr(7);
			materials[materialName] = Material();
			res.push_back(line.substr(7));
		}

		else if (line.find("map_Kd ") != line.npos) {
			std::string texturePath = directory + "/" + line.substr(7);
			getTextureId(texturePath);
			materials[materialName].texturePath = texturePath;
		}
	}
	return res;
}