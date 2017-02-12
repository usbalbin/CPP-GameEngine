#pragma once

#include "OpenClTesting\Containers.hpp"
#include "OpenClTesting\ClRayTracer.hpp"

#include "bullet/btBulletDynamicsCommon.h"
#include "glm/gtc/type_ptr.hpp"
#include "OpenClTesting\Utils.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\euler_angles.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "SFML/Audio.hpp"
#include "Entity.hpp"

class Entity;


class Shape
{
public:
	Shape();
	Shape(Entity* parent, ClRayTracer * renderer, btDiscreteDynamicsWorld* physics);

	void initialize(Instance& graphicsObject, btRigidBody* physicsObject);
	~Shape();
	virtual void update(float deltaTime);
	virtual void draw();
	virtual void draw(glm::mat4 matrix);
	void disablePhysics();

	glm::mat4 getTranslationMatrix();
	glm::mat4 getInvertedTranslationMatrix();
	glm::mat4 getMatrix();
	glm::vec3 getPosition();
	glm::vec3 getScale();
	btCollisionShape* getCollisionShape();
	virtual void calcHit(btManifoldPoint& cp) {};

	btRigidBody* physicsObject = nullptr;

	//virtual void initializeBuilder(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics) = 0;
protected:
	glm::vec3 scale;
	Entity* parent;
	static ClRayTracer* renderer;
	static btDiscreteDynamicsWorld* physics;
	sf::Sound sound;
	bool isServer();

private:
	Instance graphicsObject;
	btCollisionShape* physicsShape = nullptr;
};

