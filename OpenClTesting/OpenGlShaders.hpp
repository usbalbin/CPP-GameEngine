#pragma once
#define NOMINMAX
#define GLEW_STATIC
#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include <string>
#include <vector>
#include "Containers.hpp"

class OpenGlShaders
{
public:
	OpenGlShaders();
	//OpenGlShaders();
	OpenGlShaders::OpenGlShaders(int width, int height, bool fullScreen, std::string vertexShaderPath, std::string fragmentShaderPath, GLFWframebuffersizefun customResizeCallback);
	~OpenGlShaders();
	void resizeCallback(GLFWwindow * window, int width, int height);
	GLuint setupScreen();
	void draw();
	GLFWwindow* getWindow() { return window; }
	void writeToScreen(std::vector<float4>& colors);
	void writeToScreen(float * colors);

	

private:
	void setupWindow(GLFWframebuffersizefun customResizeCallback, bool fullScreen);
	void resizeWindow(int width, int height);
	GLuint setupShaders(std::string vertexShaderPath, std::string fragmentShaderPath);
	GLuint compileShader(std::string shaderPath, GLuint shaderType);

	GLFWwindow* window = nullptr;
	GLuint programID = 0;
	GLuint vertexBufferID = 0;
	GLuint indexBufferID = 0;
	GLuint textureBufferID;
	GLuint vertexArrayID = 0;
	GLint positionIndex = 0;
	int indexCount = 6;

	int width;
	int height;
};

