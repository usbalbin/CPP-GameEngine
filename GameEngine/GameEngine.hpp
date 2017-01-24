#pragma once

#include "Entity.hpp"

typedef std::chrono::high_resolution_clock::time_point TimePoint;

class GameEngine {
public:
	GameEngine(int width, int height);
	~GameEngine();

	void initialize();

	template<typename T>
	void initializeBuilders(T entity) {
		T::initializeBuilder(&renderer, physics);
		renderer.writeToObjectTypeBuffers();
	}

	template<typename T, typename... Ts>
	void initializeBuilders(T entity, Ts... entities) {
		T::initializeBuilder(&renderer, physics);
		initializeBuilders(entities...);
	}

	void openScene(std::string& scenePath, std::vector<Entity*>& gameEntities, Entity*& player);

	void update();
	void draw();

	bool shouldExit();
private:
	void updateTime();


	OpenClRayTracer renderer;
	
	btDiscreteDynamicsWorld* physics;
	Entity* player = nullptr;
	std::vector<Entity*> gameEntities;

	TimePoint lastTime = std::chrono::high_resolution_clock::now();
	float deltaTime;
};

