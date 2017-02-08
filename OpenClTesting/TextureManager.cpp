
#include "stdafx.h"
#include "TextureManager.hpp"

#include "Utils.hpp"

TextureManager::TextureManager(int maxTextureCount, cl::Context& context) : maxTextureCount(maxTextureCount), context(context)
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

std::string TextureManager::genOclTextureArgumentsCode()
{
	std::string res;

	if(maxTextureCount)
		res += " read_only image2d_t texture0";

	for (int i = 1; i < maxTextureCount; i++) {
		res += ", read_only image2d_t texture" + std::to_string(i);
	}
	return res;
}

std::string TextureManager::genOclTextureBlendCode()
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

void TextureManager::setTextureArguments(int argCounter, cl::Kernel & kernel)
{
	for (auto& textureBuffer : textureBuffers)
		kernel.setArg(argCounter++, textureBuffer);
}

size_t TextureManager::getTextureId(std::string & path) {
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