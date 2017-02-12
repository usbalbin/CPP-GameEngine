#pragma once

#include <set>
#include <unordered_map>
#include "CL\cl2.hpp"

struct Material {
	std::string texturePath;
};

class MaterialManager {
public:
	MaterialManager() {};
	MaterialManager(int maxTextureCount, cl::Context& context);

	std::string genOclTextureArgumentsCode();
	std::string genOclTextureBlendCode();
	void setTextureArguments(int argCounter, cl::Kernel& kernel);
	size_t getTextureId(const std::string& path);

	//Key is formated as "<MTL-path>:<material-name>"
	const Material& getMaterial(std::string materialKey) { return materials[materialKey]; }

	std::vector<std::string> readMtlFileMulti(std::string & mtlFilePath);

private:
	int maxTextureCount = 2;
	size_t currentTextureCount;
	std::vector<cl::Image2D> textureBuffers;

	//Key is formated as "<MTL-path>:<material-name>"
	std::unordered_map<std::string, Material> materials;

	//Key is texure path
	std::unordered_map<std::string, size_t> textureIds;

	//Mtl file and what materials it has defined
	std::set<std::string> loadeMtlFiles;

	cl::Context context;
};