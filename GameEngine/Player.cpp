#include "stdafx.h"
#include "Player.hpp"


Player::Player(std::vector<Entity*>& allEntities, OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw, float pitch, float roll) : Character(renderer, physics, position, yaw, pitch, roll)
	, allEntities(allEntities)
{
}

Player::~Player()
{
}

void Player::enterVehicle()
{
}

void Player::exitVehicle()
{
}
