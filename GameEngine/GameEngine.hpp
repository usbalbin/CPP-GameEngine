#pragma once

#include "GameBase.hpp"
#include "Entity.hpp"


class GameEngine : public GameBase {
public:
	GameEngine(int width, int height);
	~GameEngine();

	template<typename T>
	void initializeBuilders(T entity) {
		T::initializeBuilder(renderer, physics);
		renderer->writeToObjectTypeBuffers();
	}

	template<typename T, typename... Ts>
	void initializeBuilders(T entity, Ts... entities) {
		T::initializeBuilder(renderer, physics);
		initializeBuilders(entities...);
	}

	virtual void initialize();

	virtual void openScene(std::string fileName);

	//void openScene(std::istream & sceneStream, std::vector<Entity*>& gameEntities, Entity *& player);

	virtual void update() override;
	virtual void draw();

	bool shouldExit() override;
protected:
	void updateTime();

	OpenClRayTracer* renderer;
	Input input;
	Entity* player = nullptr;
};

