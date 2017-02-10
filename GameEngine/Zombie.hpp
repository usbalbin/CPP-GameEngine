#pragma once
#include "Player.hpp"
class Zombie : public Character
{
public:
	Zombie(std::vector<Player*>& players, ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, float yaw = 0, float pitch = 0, float roll = 0);
	~Zombie();
	void update(float deltaTime) override;
	void handleInput(const Input& input, float deltaTime) override {};
private:
	Input input;
	std::vector<Player*>& players;	//List of players to eat
};

