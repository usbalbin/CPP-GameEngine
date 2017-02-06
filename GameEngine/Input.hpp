#pragma once

#include "glm\vec2.hpp"
#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "OpenClTesting\ClRayTracer.hpp"
#include "SFML\Network.hpp"

#define NUM_INPUTS 16
#define NUM_EXTRA_INPUTS NUM_INPUTS-14

class Input
{
public:
	Input() {};
	Input(ClRayTracer* renderer);
	~Input();
	void readInput(float deltaTime);
	void clearInput();
	void readKeyBoard();
	void readMouse(float deltaTime);
	bool readGamingWheel(int joyStick = GLFW_JOYSTICK_1);

	float& operator[](const int input) { return inputs[input]; }

	ClRayTracer* renderer;

	union {
		struct {
			glm::vec2 leftStick;
			glm::vec2 rightStick;
			float leftTrigger, rightTrigger;
			float leftBumper, rightBumper;
			float buttonE, buttonR;
			float buttonShift, buttonSpace;
			float buttonF, buttonCtrl;
		};
		float inputs[NUM_INPUTS];
	};
	
};

sf::Packet& operator << (sf::Packet& packet, const Input& input);
sf::Packet& operator >> (sf::Packet& packet, Input& input);