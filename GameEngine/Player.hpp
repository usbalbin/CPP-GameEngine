#pragma once
#include "Character.hpp"
class Player :
	public Character
{
public:
	Player(std::vector<Entity*>& allEntities, OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Player();

	void enterVehicle();
	void exitVehicle();
private:
	std::vector<Entity*>& allEntities;
};

