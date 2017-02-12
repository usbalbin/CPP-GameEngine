#include "Utils.hpp"

std::string absToRelPath(std::string workingDirectory, std::string absPath) {
	for (int i = 0; i < std::min(workingDirectory.length(), absPath.length()); i++) {
		int end0 = workingDirectory.find("/\\");
		std::string a = workingDirectory.substr(0, end0);

		int end1 = workingDirectory.find("/\\");
		std::string b = workingDirectory.substr(0, end1);

		if (a != b)
			break;
		workingDirectory = workingDirectory.substr(end0 + 1);
		absPath = absPath.substr(end0 + 1);
	}

	int upCount = std::count(workingDirectory.begin(), workingDirectory.end(), '/') +
		std::count(workingDirectory.begin(), workingDirectory.end(), '\\');

	std::string result = "";
	for (int i = 0; i < upCount; i++)
		result += "../";

	result += absPath;
	return result;
}

