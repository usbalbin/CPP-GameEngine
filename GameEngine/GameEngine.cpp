// GameEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GameEngine.hpp"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "OpenClTesting\OpenClRayTracer.hpp"
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
#include "Character.hpp"
#include "Terrain.hpp"
#include "Cannon.hpp"
#include "RifleTester.hpp"


#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Sphere.hpp"


GameEngine::GameEngine(int width, int height) : renderer(width, height) {
	renderer.initializeAdvancedRender();

	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	physics = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	physics->setGravity(btVector3(0, -9.82f, 0));
}

GameEngine::~GameEngine() {
	for (auto& entity : gameEntities)
		delete entity;

	auto broadphase = physics->getBroadphase();
	auto dispatcher = physics->getDispatcher();
	auto solver = physics->getConstraintSolver();


	delete physics;
	delete solver;
	delete dispatcher;
	//delete collisionConfiguration;
	delete broadphase;

}

void GameEngine::initialize() {
	initializeBuilders(Cube(), Cylinder(), Sphere());

	openScene(std::string("content/scene.scene"), gameEntities, player);
}

void GameEngine::openScene(std::string & scenePath, std::vector<Entity*>& gameEntities, Entity *& player) {
	std::ifstream file(scenePath);


	

	std::string str;
	for (int row = 0; getline(file, str); row++) {
		str = str.substr(0, str.find("//"));//Strip out comments

		std::stringstream line(str);

		std::string objectType;
		float x = 0, y = 0, z = 0;

		line >> objectType >> x >> y >> z;

		if (!objectType.size()) 
			continue;
		

		Entity* entity;

		if (contains({ "box", "sphere" }, objectType)) {
			float scaleX = 0, scaleY = 0, scaleZ = 0, mass = 0;
			line >> scaleX >> scaleY >> scaleZ >> mass;

			if (objectType == "box") {
				entity = new Box(&renderer, physics, glm::vec3(x, y, z), glm::vec3(scaleX, scaleY, scaleZ), mass);
			}
			else if (objectType == "ball")							 //Radius, mass
				entity = new Ball(&renderer, physics, glm::vec3(x, y, z), scaleX, scaleY);

		}
		else {

			float yaw = 0;
			line >> yaw;

			if (objectType == "character")
				entity = new Character(&renderer, physics, glm::vec3(x, y, z), yaw);


			else if (objectType == "tank")
				entity = new Tank(&renderer, physics, glm::vec3(x, y, z), yaw);
			else if (objectType == "apc")
				entity = new ApcTank(&renderer, physics, glm::vec3(x, y, z), yaw);
			else if (objectType == "car")
				entity = new Car(&renderer, physics, glm::vec3(x, y, z), yaw);

			else
				throw(std::string("Invalid syntax at line: ") + std::to_string(row) + std::string("\n") + line.str());

		}
		gameEntities.push_back(entity);
		if (!player)//Set player to first entity
			player = entity;
	}
}

void GameEngine::update()
{
	updateTime();
	player->handleInput(deltaTime);

	for (auto entity : gameEntities)
		entity->update(deltaTime);

	physics->stepSimulation(deltaTime / 1.0f, 10);

	
}

void GameEngine::draw()
{
	static float yaw = 0, pitch = 0;

	renderer.clear();

	for (auto entity : gameEntities)
		entity->draw();

	if (glfwGetKey(renderer.getWindow(), GLFW_KEY_KP_4) == GLFW_PRESS)
		yaw -= deltaTime * 0.4f;
	if (glfwGetKey(renderer.getWindow(), GLFW_KEY_KP_6) == GLFW_PRESS)
		yaw += deltaTime * 0.4f;

	if (glfwGetKey(renderer.getWindow(), GLFW_KEY_KP_8) == GLFW_PRESS)
		pitch += deltaTime * 0.4f;
	if (glfwGetKey(renderer.getWindow(), GLFW_KEY_KP_2) == GLFW_PRESS)
		pitch -= deltaTime * 0.4f;

	if (glfwGetKey(renderer.getWindow(), GLFW_KEY_KP_5) == GLFW_PRESS)
		yaw = pitch = 0;


	glm::vec3 relativeCameraPos(0, 1.3f, 0);
	//Character fps camera
	//glm::mat4 cameraMatrix = toMatrix(glm::vec3(0, 0.1f, -0.1f)) * glm::rotate(glm::rotate(glm::mat4(1), pitch, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * toMatrix(relativeCameraPos) * player->getTranslationMatrix();

	//Tank 3ps camera
	glm::mat4 cameraMatrix = toMatrix(glm::vec3(0, 0, 15)) * glm::rotate(glm::rotate(glm::mat4(1), pitch + 1.1f * PI_HALF, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * player->getTranslationMatrix();


	//glm::mat4 cameraMatrix = toMatrix(glm::vec3(-0.5f, -0.2f, 1)) * glm::rotate(glm::rotate(glm::mat4(1), pitch + 1.0f * PI_HALF, glm::vec3(1, 0, 0)), yaw, glm::vec3(0, 1.0f, 0)) * player->getTranslationMatrix();

	renderer.autoResize();
	renderer.render(cameraMatrix);
}

bool GameEngine::shouldExit()
{
	return glfwWindowShouldClose(renderer.getWindow());
}

void GameEngine::updateTime() {
	auto nowTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration<float>(nowTime - lastTime).count();
	lastTime = nowTime;
}