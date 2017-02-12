#include "stdafx.h"
#include "Zombie.hpp"

#include <set>


Zombie::Zombie(std::vector<Player*>& players, ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw, float pitch, float roll) :
	Character(renderer, physics, position, yaw, pitch, roll), players(players)
{
}



Zombie::~Zombie()
{
}

void Zombie::update(float deltaTime) {
	Character::update(deltaTime);

	if (isDead())
		return handleInput(input, deltaTime);

	const float PI = 3.14159265;

	std::function<bool(Player*, Player*)> isCloser = [&](Player* left, Player* right) {
		return glm::distance2(left->getPosition(), this->getPosition()) < glm::distance2(right->getPosition(), this->getPosition());
	};

	std::function<bool(Player*)> isPotentialVictim = [&](Player* player) {
		float distance2 = glm::distance2(player->getPosition(), this->getPosition());

		float distance2Limit = 25 * 25;

		return distance2 < distance2Limit;
	};


	std::set<Player*, std::function<bool(Player*, Player*)>> potentialVictims(isCloser);
	std::copy_if(players.begin(), players.end(), std::inserter(potentialVictims, potentialVictims.end()), isPotentialVictim);

	if (potentialVictims.empty()) {
		input.clearInput();
	}
	else {
		Player* closestMeal = *potentialVictims.begin();

		auto deltaPos = closestMeal->getPosition() - this->getPosition();

		float preferedYaw = PI + atan2(deltaPos.x, deltaPos.z);
		float preferedPitch = -atan2(deltaPos.y, glm::length(glm::vec2(deltaPos.x, deltaPos.z)));

		float currentYaw = this->getYaw();
		float currentPitch = this->getPitch();

		float deltaYaw = btNormalizeAngle(preferedYaw - currentYaw);
		float deltaPitch = btNormalizeAngle(preferedPitch - currentPitch);
		input.rightStick.x = 10 * deltaYaw;
		input.rightStick.y = 10 * deltaPitch;
		btClamp(input.rightStick.x, -1.0f, +1.0f);
		btClamp(input.rightStick.y, -1.0f, +1.0f);

		//input.leftBumper = glm::length2(glm::vec2(deltaYaw, deltaPitch)) < PI * 0.0002f; //Fire if meal in sight

		input.leftStick.y = cosf(deltaYaw);//walk towards meal



		float distance2 = glm::distance2(closestMeal->getPosition(), this->getPosition());
		float eatRange2 = 1 * 1;

		if (distance2 < eatRange2) {//Player is close enough to eat
			float damagePerSec = 0.5f;
			//head->calcHit(btManifoldPoint());
			closestMeal->damage(deltaTime * damagePerSec);
		}
	}

	Character::handleInput(input, deltaTime);
}

bool Zombie::justDied()
{
	input.clearInput();
	Character::handleInput(input, 1);

	return Character::justDied();
}
