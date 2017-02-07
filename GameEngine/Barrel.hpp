#pragma once
#include "Shape.hpp"
#include "Cylinder.hpp"
#include "Sphere.hpp"

#include "Input.hpp"

#include <queue>
#include <chrono>

using namespace std::chrono_literals;

enum FireMode{
	SAFE,
	SINGLE,
	FULL
};

enum TriggerState : bool{
	RELEASED = false,
	PULLED = true
};

struct Projectile {
	
	Projectile(Sphere* sphere, std::chrono::duration<double> ttl)
		: sphere(sphere), ttl(ttl),
		birthTime(std::chrono::system_clock::now()){
		sphere->physicsObject->setCcdMotionThreshold(sphere->getRadius());
		sphere->physicsObject->setCcdSweptSphereRadius(sphere->getRadius() * 0.2f);
	}
	~Projectile() {
		delete sphere;
	}

	bool isTimeToDie(std::chrono::system_clock::time_point now) { 
		return now - birthTime >= ttl;
	}

	void draw() { sphere->draw(); }
	Sphere* sphere;
	std::chrono::system_clock::time_point birthTime;
	std::chrono::duration<double> ttl;
};

class Barrel :
	public Cylinder
{
public:
	Barrel(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics, glm::vec3 position, glm::vec2 scale, float mass, float projectileRadius, float projectileMass, float yaw, float pitch, float roll, std::chrono::duration<double> fireRate, std::vector<FireMode> fireModes, std::string soundFile);
	~Barrel();
	void draw();
	void draw(glm::mat4 matrix);
	void update(float deltaTime);


	//Default to its own physics object if none is specified
	void updateBarrel(const Input& input, int fireKey, btRigidBody * physicsObject = nullptr, glm::vec3 recoilCenter = glm::vec3(0), glm::mat4 barrelTransMatrix = glm::mat4(1));
private:
	float calcPropellingImpulse();
	float calcRecoilImpulse();

	std::vector<FireMode> fireModes;
	FireMode currentFireMode;
	float projectileRadius, projectileMass;

	std::deque<Projectile> projectiles;
	std::chrono::system_clock::time_point lastFired;
	std::chrono::duration<double> fireRate;
	TriggerState lastTriggerState = TriggerState::RELEASED;
	sf::SoundBuffer firingSound;
};

