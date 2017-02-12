#pragma once

#include "Entity.hpp"
#include "Options.hpp"
#include <deque>

#include "Player.hpp"

#include "Garbage.hpp"

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
	void openScene(std::istream & sceneStream, Entity ** player = nullptr, ClRayTracer * renderer = nullptr);
	void addEntity(Entity* entity);
	void removeEntity(Entity* entity);
	void updateGarbage();
	virtual void onGarbageRemove(Entity* entity) {};
	std::vector<Entity*>& getGameEntities() { return gameEntities; }
	void updateTime();
	const std::vector<Entity*> getGameEntities() const { return gameEntities; }

	btDiscreteDynamicsWorld* physics;
	

	RtTimePoint lastTime = std::chrono::high_resolution_clock::now();
	float deltaTime;
	Options options;
	std::vector<Player*> players;

private:
	static bool collisionCallback(btManifoldPoint& cp, void* body0, void* body1);
	std::vector<Entity*> gameEntities;
	std::deque<Garbage> garbageEntities;
};

