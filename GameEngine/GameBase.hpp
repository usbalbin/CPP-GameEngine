#pragma once

#include "Entity.hpp"

//typedef std::chrono::high_resolution_clock::time_point TimePoint;

class GameBase {
public:
	GameBase();
	~GameBase();

	virtual void initialize()=0;

	//void openScene(std::istream & sceneStream, std::vector<Entity*>& gameEntities, Entity *& player);


	virtual void update();

	virtual bool shouldExit();
protected:
	void openScene(std::istream & sceneStream, Entity ** player = nullptr, OpenClRayTracer * renderer = nullptr);
	void addEntity(Entity* entity);
	void updateTime();
	const std::vector<Entity*> getGameEntities() const { return gameEntities; }

	btDiscreteDynamicsWorld* physics;
	

	RtTimePoint lastTime = std::chrono::high_resolution_clock::now();
	float deltaTime;
private:
	std::vector<Entity*> gameEntities;
};

