#include "stdafx.h"
#include "Options.hpp"

#include "Utils.hpp"





/*
int Options::width;
int Options::height;
bool Options::forceCpu;
bool Options::fullScreen;

int Options::serverPort;
std::string Options::serverAddress;
std::string Options::scenarioPath;
std::string Options::playerName;
*/












Options::Options(std::string optionsPath, bool setDefaultValues)
{
	load(optionsPath, setDefaultValues);
}

Options::Options(std::istream & optionStream, bool setDefaultValues)
{
	load(optionStream, setDefaultValues);
}

void Options::save(std::string optionsPath)
{
	std::ofstream optionsFile(optionsPath);
	save(optionsFile);
}

void Options::save(std::ostream& optionStream) {
	optionStream << "width " << width << std::endl;
	optionStream << "height " << height << std::endl;
	optionStream << "forceCpu " << (int)forceCpu << std::endl;
	optionStream << "fullScreen " << (int)fullScreen << std::endl;
	optionStream << "serverAddress " << serverAddress << std::endl;
	optionStream << "scenarioPath " << scenarioPath << std::endl;
	optionStream << "playerName " << playerName << std::endl;
}

void Options::setDefaults() {
	width = 1024;
	height = 768;
	forceCpu = false;
	fullScreen = false;
	serverPort = 6536;
	serverAddress = "localhost";
	scenarioPath = "content/scene.scene";
	playerName = "Player";
}

void Options::load(std::string optionsPath, bool setDefaultValues)
{
	std::ifstream optionsFile(optionsPath);
	load(optionsFile, setDefaultValues);
}



void Options::load(std::istream& optionStream, bool setDefaultValues){
	if(setDefaultValues)
		setDefaults();


	std::string str;
	for (int row = 0; getline(optionStream, str); row++) {
		str = str.substr(0, str.find("//"));//Strip out comments

		std::stringstream line(str);

		std::string optionType;
		line >> optionType;

		if (!optionType.size())
			continue;

		if (contains({ "width", "height", "forceCpu", "fullScreen", "serverPort" }, optionType)) {//Integer types
			int data;
			line >> data;

			if (optionType == "width")
				width = data;
			else if (optionType == "height")
				height = data;
			else if (optionType == "forceCpu")
				forceCpu = data;
			else if (optionType == "fullScreen")
				fullScreen = data;
			else if (optionType == "serverPort")
				serverPort = data;
		}

		else if (contains({ "serverAddress", "scenarioPath", "playerName" }, optionType)) {//String types
			std::string data;
			line >> data;

			if (optionType == "serverAddress")
				serverAddress = data;
			else if (optionType == "scenarioPath")
				scenarioPath = data;
			else if (optionType == "playerName")
				playerName = data;
		}
		else {
			throw(std::string("Invalid syntax at line: ") + std::to_string(row) + std::string("\n") + line.str());
		}
	}
}