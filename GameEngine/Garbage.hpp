#pragma once

#include "Entity.hpp"

class Garbage
{
public:

	Garbage(Entity* entity, std::chrono::duration<double> ttl)
		: entity(entity), ttl(ttl),
		birthTime(std::chrono::system_clock::now()) {}
	~Garbage() {
	}

	bool isTimeToDie(std::chrono::system_clock::time_point now) {
		return now - birthTime >= ttl;
	}

	Entity* entity;
	std::chrono::system_clock::time_point birthTime;
	std::chrono::duration<double> ttl;
};

