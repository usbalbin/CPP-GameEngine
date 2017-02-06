#pragma once

#include <fstream>
#include <sstream>


class Options
{
public:
	Options(std::string optionsPath, bool setDefaultValues = true);
	Options(std::istream& optionStream, bool setDefaultValues = true);
	void save(std::string optionsPath);
	void save(std::ostream& optionStream);
	void setDefaults();
	void load(std::string optionsPath, bool setDefaultValues = true);
	void load(std::istream& optionStream, bool setDefaultValues = true);







	int width;
	int height;
	bool forceCpu;
	bool fullScreen;

	int serverPort;
	std::string serverAddress;
	std::string scenarioPath;
	std::string playerName;

};

