#pragma once
#include "Character.hpp"

#include "Vehicle.hpp"

class Player :
	public Character
{
public:
	Player(std::vector<Entity*>& allEntities, OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Player();

	void draw();
	void handleInput(float deltaTime);
private:

	glm::mat4 getMatrix();

	glm::mat4 getTranslationMatrix();


	bool lastEnterVehicleKeyPressed = false;
	bool isInVehicle();
	void enterVehicle();
	void exitVehicle();

	Vehicle* vehicle = nullptr;
	std::vector<Entity*>& allEntities;
};