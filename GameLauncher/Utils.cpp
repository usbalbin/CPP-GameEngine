#include "Utils.hpp"

std::string absToRelPath(std::string workingDirectory, std::string absPath) {
	std::replace(workingDirectory.begin(), workingDirectory.end(), '\\', '/');
	std::replace(absPath.begin(), absPath.end(), '\\', '/');
	workingDirectory += workingDirectory.back() != '/' ? "/" : "";
	while(std::min(workingDirectory.length(), absPath.length())) {
		int end0 = workingDirectory.find("/");
		std::string a = workingDirectory.substr(0, end0);

		int end1 = absPath.find("/");
		std::string b = absPath.substr(0, end1);

		if (a != b)
			break;
		workingDirectory = workingDirectory.substr(end0 + 1);
		absPath = absPath.substr(end0 + 1);
	}

	int upCount = std::count(workingDirectory.begin(), workingDirectory.end(), '/');

	std::string result = "";
	for (int i = 0; i < upCount; i++)
		result += "../";

	result += absPath;
	return result;
}

