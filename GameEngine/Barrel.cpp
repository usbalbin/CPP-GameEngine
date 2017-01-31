#include "stdafx.h"
#include "Barrel.hpp"

#include "Utils.hpp"
#include <fstream>

Barrel::Barrel(OpenClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, glm::vec2 scale, float mass, float projectileRadius, float projectileMass, float yaw, float pitch, float roll, std::chrono::duration<double> fireRate, std::vector<FireMode> fireModes) :
	Cylinder(renderer, physics, position, scale, mass, yaw, pitch, roll)
{
	this->projectileRadius = projectileRadius;
	this->projectileMass = projectileMass;
	this->lastFired = std::chrono::system_clock::now();
	this->fireRate = fireRate;
	this->fireModes = fireModes;
	this->currentFireMode = fireModes[0];
	
	if (renderer) {
		if (!firingSound.loadFromFile("content/TankShot.wav"))
			throw "Sound did'nt get loaded correctly Content/TankShot.wav";
		sound.setBuffer(firingSound);
	}
}


Barrel::~Barrel()
{
	
}

void Barrel::draw() {
	Cylinder::draw();
	for (auto& projectile : projectiles)
		projectile.draw();
}

void Barrel::draw(glm::mat4 matrix) {
	Cylinder::draw(matrix);
	for (auto& projectile : projectiles)
		projectile.draw();
}

void Barrel::update(float deltaTime) {
	Cylinder::update(deltaTime);
	auto now = std::chrono::system_clock::now();
	while (projectiles.size() && projectiles.front().isTimeToDie(now))
		projectiles.pop_front();
	if(renderer && physicsObject)
		sound.setPosition(getPosition().x, getPosition().y, getPosition().z);
}

void Barrel::updateBarrel(const Input& input, int fireKey, btRigidBody* physicsObject, glm::vec3 recoilCenter, glm::mat4 barrelTransMatrix) {
	TriggerState triggerState = (TriggerState)(int)input.inputs[fireKey];
	auto deltaTime = std::chrono::system_clock::now() - lastFired;
	if (
		currentFireMode == FireMode::SAFE ||
		deltaTime < fireRate ||
		triggerState == TriggerState::RELEASED
	) {
		lastTriggerState = triggerState;
		return;
	}

	if (currentFireMode == lastTriggerState)
		return;
	
	if(renderer)
		sound.play();
	barrelTransMatrix = !physicsObject ? getTranslationMatrix() : barrelTransMatrix;
	glm::mat4 invBarrelTransMatrix = glm::inverse(barrelTransMatrix);


	glm::vec3 projectilePos = /*toVec3(barrelTransMatrix) + */glm::vec3(glm::vec4(0, -scale.y - projectileRadius, 0, 1) * barrelTransMatrix);
	Sphere* projectile = new Sphere(renderer, physics, projectilePos, projectileRadius, projectileMass);

	
	
	glm::vec3 propellingVector(invBarrelTransMatrix * glm::vec4(0, -calcPropellingImpulse(), 0, 0));
	glm::vec3 recoilVector(invBarrelTransMatrix * glm::vec4(0, +calcRecoilImpulse(), 0, 0));
	projectile->physicsObject->applyCentralImpulse(toVector3(propellingVector));
	
	if (!physicsObject) {//Default to its own physics object if none is specified
		physicsObject = this->physicsObject;
	}
	physicsObject->applyImpulse(toVector3(recoilVector), toVector3(recoilCenter));

	projectiles.emplace_back(projectile, 5.0s);

	lastTriggerState = triggerState;
	lastFired += deltaTime;
}

float Barrel::calcPropellingImpulse()
{
	const float muzzleVelocity = 400;

	return (projectileMass * muzzleVelocity);
}

//http://www.alternatewars.com/BBOW/Ballistics/Gun_Recoil.htm
float Barrel::calcRecoilImpulse()
{
	const float muzzleVelocity = 400;
	const float powderMass = 2.0f * projectileMass;

	//Ratio between projectiles muzzle velocity and powder gases muzzle velocity
	float magicCoefficient = 576.17f * pow(muzzleVelocity, -0.877f);
	
	//Muzzle velocity of powder gases
	float powderVelocity = muzzleVelocity / magicCoefficient;

	return calcPropellingImpulse() + (powderMass * powderVelocity);
}

