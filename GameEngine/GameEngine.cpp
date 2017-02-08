#include "stdafx.h"

#include "GameEngine.hpp"

#include "SFML/Audio.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>


#include "OpenClTesting\ClRayTracer.hpp"
#include "OpenClTesting\OpenClContexts.hpp"
#include "OpenClTesting\OpenGlShaders.hpp"
#include "OpenClTesting\Meshes.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "OpenClTesting\Utils.hpp"

#include "Utils.hpp"

#include "SimpleTerrain.hpp"
#include "Car.hpp"
#include "ApcTank.hpp"
#include "Tank.hpp"
#include "Box.hpp"
#include "Ball.hpp"
#include "Player.hpp"
#include "Terrain.hpp"
#include "Cannon.hpp"
#include "TestEntity.hpp"

#include "Options.hpp"

#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Sphere.hpp"



GameEngine::GameEngine() : renderer(new ClRayTracer(options.width, options.height, options.forceCpu, options.fullScreen)), input(renderer) {
	renderer->initializeAdvancedRender();
}

GameEngine::~GameEngine() {
	delete renderer;
}

void GameEngine::initialize() {
	Vertex v;
	int d = (float*)&v.uv - (float*)&v.position;
	int s = sizeof(Vertex);

	openScene(options.scenarioPath);
//	gameEntities.push_back(
//		new Terrain(&renderer, physics)
//	);
	/*
	addEntity(
		new TestEntity(renderer, physics, glm::vec3(0, 100, 0))
	);*/
}

void GameEngine::openScene(std::string fileName)
{
	std::ifstream sceneStream(fileName);
	GameBase::openScene(sceneStream, &player, renderer);
}

void GameEngine::update()
{
	updateTime();
	
	input.readInput(deltaTime);
	player->handleInput(input, deltaTime);

	GameBase::update();
}

void GameEngine::draw()
{
	static float yaw = 0, pitch = 0;

	renderer->clear();

	for (auto entity : getGameEntities())
		entity->draw();

	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_KP_4) == GLFW_PRESS)
		yaw -= deltaTime * 0.4f;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_KP_6) == GLFW_PRESS)
		yaw += deltaTime * 0.4f;

	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_KP_8) == GLFW_PRESS)
		pitch += deltaTime * 0.4f;
	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_KP_2) == GLFW_PRESS)
		pitch -= deltaTime * 0.4f;

	if (glfwGetKey(renderer->getWindow(), GLFW_KEY_KP_5) == GLFW_PRESS)
		yaw = pitch = 0;


	glm::vec3 relativeCameraPos(0, 0, 0);
	//Character fps camera
	//glm::mat4 cameraMatrix = toMatrix(glm::vec3(0, 0.1f, -0.21f)) * glm::rotate(glm::rotate(glm::mat4(1), pitch, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * toMatrix(relativeCameraPos) * player->getTranslationMatrix();

	//Tank 3ps camera
	//glm::mat4 cameraMatrix = toMatrix(glm::vec3(0, 0, 5)) * glm::rotate(glm::rotate(glm::mat4(1), pitch + 1.1f * PI_HALF, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * player->getTranslationMatrix();


	//glm::mat4 cameraMatrix = toMatrix(glm::vec3(-0.5f, -0.2f, 1)) * glm::rotate(glm::rotate(glm::mat4(1), pitch + 1.0f * PI_HALF, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * player->getTranslationMatrix();

	glm::mat4 cameraMatrix = player->cameraMatrix(yaw, pitch);

	sf::Listener::setUpVector(toSfVector3(glm::vec4(0, 1, 0, 0) * cameraMatrix));
	sf::Listener::setDirection(toSfVector3(glm::vec4(0, 0, -1.0f, 0) * cameraMatrix));
	sf::Listener::setPosition(toSfVector3(cameraMatrix));
	
	//renderer->autoResize();
	renderer->render(cameraMatrix);
}

bool GameEngine::shouldExit()
{
	return glfwWindowShouldClose(renderer->getWindow());
}

void GameEngine::updateTime() {
	auto nowTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration<float>(nowTime - lastTime).count();
	lastTime = nowTime;
}