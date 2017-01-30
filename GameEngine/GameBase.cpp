#include "stdafx.h"

#include "GameBase.hpp"

#include "SFML/Audio.hpp"
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
#include "Player.hpp"
#include "Terrain.hpp"
#include "Cannon.hpp"
#include "RifleTester.hpp"


#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Sphere.hpp"


GameBase::GameBase() {
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	physics = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	physics->setGravity(btVector3(0, -9.82f, 0));
}

GameBase::~GameBase() {
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

void GameBase::openScene(std::istream & sceneStream, Entity ** player, OpenClRayTracer * renderer) {


	std::string str;
	for (int row = 0; getline(sceneStream, str); row++) {
		str = str.substr(0, str.find("//"));//Strip out comments

		std::stringstream line(str);

		std::string objectType;
		float x = 0, y = 0, z = 0;

		line >> objectType >> x >> y >> z;

		if (!objectType.size())
			continue;


		Entity* entity = nullptr;

		if (contains({ "box", "sphere" }, objectType)) {
			float scaleX = 0, scaleY = 0, scaleZ = 0, mass = 0;
			line >> scaleX >> scaleY >> scaleZ >> mass;

			if (objectType == "box") {
				entity = new Box(renderer, physics, glm::vec3(x, y, z), glm::vec3(scaleX, scaleY, scaleZ), mass);
			}
			else if (objectType == "ball")							 //Radius, mass
				entity = new Ball(renderer, physics, glm::vec3(x, y, z), scaleX, scaleY);

		}
		else {

			float yaw = 0;
			line >> yaw;

			if (objectType == "character")
				entity = new Player(gameEntities, renderer, physics, glm::vec3(x, y, z), yaw);


			else if (objectType == "tank")
				entity = new Tank(renderer, physics, glm::vec3(x, y, z), yaw);
			else if (objectType == "apc")
				entity = new ApcTank(renderer, physics, glm::vec3(x, y, z), yaw);
			else if (objectType == "car")
				entity = new Car(renderer, physics, glm::vec3(x, y, z), yaw);

			else
				throw(std::string("Invalid syntax at line: ") + std::to_string(row) + std::string("\n") + line.str());

		}
		addEntity(entity);
		if (player && !*player)//Set player to first entity
			*player = entity;
	}
}

void GameBase::addEntity(Entity * entity)
{
	int gameArrayIndex = gameEntities.size();
	gameEntities.push_back(entity);
	entity->setArrayIndex(gameArrayIndex);
}

bool GameBase::shouldExit()
{
	return false;
}

void GameBase::update() {
	for (auto entity : gameEntities)
		entity->update(deltaTime);

	physics->stepSimulation(deltaTime / 1.0f);
}

void GameBase::updateTime() {
	auto nowTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration<float>(nowTime - lastTime).count();
	lastTime = nowTime;
}