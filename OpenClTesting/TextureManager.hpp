#pragma once

#include <unordered_map>
#include "CL\cl2.hpp"

class TextureManager {
public:
	TextureManager() {};
	TextureManager(int maxTextureCount, cl::Context& context);

	std::string genOclTextureArgumentsCode();
	std::string genOclTextureBlendCode();
	void setTextureArguments(int argCounter, cl::Kernel& kernel);
	size_t getTextureId(std::string& path);

private:
	int maxTextureCount = 2;
	size_t currentTextureCount;
	std::vector<cl::Image2D> textureBuffers;
	std::unordered_map<std::string, size_t> textureIds;

	cl::Context context;
};