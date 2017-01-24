#pragma once

#include "glm\vec2.hpp"
#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "OpenClTesting\OpenClRayTracer.hpp"

#define NUM_INPUTS 16
#define NUM_EXTRA_INPUTS NUM_INPUTS-14

class Input
{
public:
	Input(OpenClRayTracer* renderer);
	~Input();
	void readInput(float deltaTime);
	void clearInput();
	void readKeyBoard();
	void readMouse(float deltaTime);
	bool readGamingWheel(int joyStick = GLFW_JOYSTICK_1);

private:

	float& operator[](int input) { return inputs[input]; }

	OpenClRayTracer* renderer;

	union {
		struct {
			glm::vec2 leftStick;
			glm::vec2 rightStick;
			float leftTrigger, rightTrigger;
			float leftBumper, rightBumper;
			float buttonE, buttonR;
			float buttonQ, buttonSpace;
			float buttonF, buttonCtrl;
			float extraInputs[NUM_EXTRA_INPUTS];
		};
		float inputs[NUM_INPUTS];
	};
	
};

