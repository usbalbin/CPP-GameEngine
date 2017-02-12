#include "stdafx.h"
#include "Ak47.hpp"

#include "Cube.hpp"
#include "Utils.hpp"

Ak47::Ak47(ClRayTracer* renderer, btDiscreteDynamicsWorld* physics, glm::vec3 position, float yaw, float pitch, float roll) : CompoundShape(renderer, physics)
{
	glm::vec3 scale(1);
	glm::mat4 matrix =
		glm::translate(glm::mat4(1.0f), position)

		*

		glm::scale(
			glm::yawPitchRoll(yaw + PI_HALF, pitch, roll),
			scale
		)
		;
	this->scale = scale;

	

	Instance instance = renderer->makeInstance("content/Weapon/AK47/AK47.obj");


	btCompoundShape* rifleShape = new btCompoundShape();

	float buttMass = 0.1f;
	glm::vec3 buttHalfExtent(0.015f, 0.05f, 0.02f);
	glm::vec3 buttPos = glm::vec3(0, -buttHalfExtent.y, -buttHalfExtent.z);
	CompoundShapeChild* butt = new CompoundShapeChild(
		renderer, physics, toTransform(buttPos),
		new Cube(renderer, physics, buttPos, buttHalfExtent, 0),
		physicsObject
	);
	addChild(butt);

	float combMass = 0.2f;
	glm::vec3 combHalfExtent(0.025f, 0.03f, 0.1f);
	glm::vec3 combPos = buttPos + glm::vec3(0, buttHalfExtent.y, -buttHalfExtent.z - combHalfExtent.z);
	CompoundShapeChild* comb = new CompoundShapeChild(
		renderer, physics, toTransform(combPos),
		new Cube(renderer, physics, combPos, combHalfExtent, 0),
		physicsObject
	);
	addChild(comb);

	float baseMass = 0.5f;
	glm::vec3 baseHalfExtent(0.03f, 0.05f, 0.1f);
	glm::vec3 basePos = combPos + glm::vec3(0, 0, -combHalfExtent.z - baseHalfExtent.z);
	CompoundShapeChild* base = new CompoundShapeChild(
		renderer, physics, toTransform(basePos),
		new Cube(renderer, physics, basePos, baseHalfExtent, 0),
		physicsObject
	);
	addChild(base);

	

	float bulletDiameter = 0.1f;//0.00782;
	float bulletMass = 0.010;
	auto fireRate = 60.0s / 800;
	std::vector<FireMode> fireModes{ /*FireMode::SINGLE,*/ FireMode::FULL };


	float barrelMass = 0.5f;
	glm::vec2 barrelHalfExtent(0.01f, 0.1f);
	glm::vec3 barrelPos = basePos + glm::vec3(0, 0, -baseHalfExtent.z - barrelHalfExtent.y);

	barrel = new CompoundShapeChild(
		renderer, physics, toTransform(barrelPos, 0, PI_HALF, 0),
		new Barrel(renderer, physics, barrelPos, barrelHalfExtent, barrelMass, bulletDiameter, bulletMass, 0, PI_HALF, 0, fireRate, fireModes, "content/Weapon/AK47/AK47.wav"),
		physicsObject
	);
	addChild(barrel);


	btTransform transform = toTransform(position);
	btDefaultMotionState* motionState = new btDefaultMotionState(transform);

	std::vector<float> masses{ buttMass, combMass, baseMass, barrelMass };
	finalize(position, masses, yaw, pitch, roll);
	initialize(instance, nullptr);
}


Ak47::~Ak47()
{
}

void Ak47::draw()
{
	Shape::draw();
	CompoundShape::draw();
}

void Ak47::update(float deltaTime)
{
	Barrel* barrelPart = ((Barrel*)barrel->getShape());
	barrelPart->update(deltaTime);
}

void Ak47::handleInput(const Input & input, float deltaTime)
{
	Barrel* barrelPart = ((Barrel*)barrel->getShape());
	barrelPart->updateBarrel(input, 6, physicsObject, toVec3(barrel->getLocalTransform().getOrigin()), barrel->getWorldMatrix());
}