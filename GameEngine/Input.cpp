#include "stdafx.h"
#include "Input.hpp"

#include "OpenClTesting\Utils.hpp"



Input::Input(OpenClRayTracer * renderer)
{
	this->renderer = renderer;
}

Input::~Input()
{
}

void Input::readInput(float deltaTime){
	clearInput();
	readKeyBoard();
	readGamingWheel(GLFW_JOYSTICK_1);


	for (int i = 4; i < NUM_INPUTS; i++)
		btClamp(inputs[i], 0.0f, 1.0f);
}

void Input::clearInput()
{
	std::fill(inputs, inputs + NUM_INPUTS, 0);
}

void Input::readKeyBoard(){
	leftStick.x += glfwGetKey(renderer->getWindow(), GLFW_KEY_D) - glfwGetKey(renderer->getWindow(), GLFW_KEY_A);
	leftStick.y += glfwGetKey(renderer->getWindow(), GLFW_KEY_W) - glfwGetKey(renderer->getWindow(), GLFW_KEY_S);

	leftBumper	+= glfwGetKey(renderer->getWindow(), GLFW_KEY_SPACE);


	buttonE += glfwGetKey(renderer->getWindow(), GLFW_KEY_E);
	buttonR += glfwGetKey(renderer->getWindow(), GLFW_KEY_R);
	buttonQ += glfwGetKey(renderer->getWindow(), GLFW_KEY_Q);
	buttonF += glfwGetKey(renderer->getWindow(), GLFW_KEY_F);
	buttonCtrl += glfwGetKey(renderer->getWindow(), GLFW_KEY_LEFT_CONTROL);
	buttonSpace += glfwGetKey(renderer->getWindow(), GLFW_KEY_SPACE);
}

void Input::readMouse(float deltaTime){
	static double lastPosX = 0, lastPosY = 0;
	double posX = 0, posY = 0;
	glfwGetCursorPos(renderer->getWindow(), &posX, &posY);


#ifdef _DEBUG
	if (glfwGetMouseButton(renderer->getWindow(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
#endif
		rightStick.x += (posX - lastPosX) * deltaTime;
		rightStick.y += (posY - lastPosY) * deltaTime;
#ifdef _DEBUG
	}
#else
	glfwSetCursorPos(renderer->getWindow(), 0, 0);
#endif

	lastPosX = posX;
	lastPosY = posY;



	leftBumper  += glfwGetMouseButton(renderer->getWindow(), GLFW_MOUSE_BUTTON_1);
	rightBumper += glfwGetMouseButton(renderer->getWindow(), GLFW_MOUSE_BUTTON_3);
}

bool Input::readGamingWheel(int joyStick) {
	int present = glfwJoystickPresent(joyStick);
	float wheel = 0;
	float throttle = 0;
	float brake = 0;

	
	
	if (present) {
		int count;
		const float* axes = glfwGetJoystickAxes(joyStick, &count);

		wheel = mapToRange(axes[0], -1.0f, +1.0f, -1.0f, +1.0f);
		throttle = mapToRange(axes[1], +1.0f, -1.0f, 0.0f, 1.0f);
		brake = mapToRange(axes[2], +1.0f, -1.0f, 0.0f, 1.0f);
	}
	
	rightTrigger += throttle;
	leftTrigger += brake;
	leftStick.y += (throttle - brake);
	leftStick.x += wheel;

	return present;
}