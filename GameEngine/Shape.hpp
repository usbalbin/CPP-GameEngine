#pragma once

#include "OpenClTesting\Containers.hpp"
#include "OpenClTesting\OpenClRayTracer.hpp"

#include "btBulletDynamicsCommon.h"
#include "glm/gtc/type_ptr.hpp"
#include "OpenClTesting\Utils.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\euler_angles.hpp"
#include "glm\gtc\matrix_transform.hpp"

class Shape
{
public:
	Shape();
	Shape(OpenClRayTracer * renderer, btDiscreteDynamicsWorld* physics);

	void initialize(MultiInstance& graphicsObject, btRigidBody* physicsObject);
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

	btRigidBody* physicsObject = nullptr;

	//virtual void initializeBuilder(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics) = 0;
protected:
	glm::vec3 scale;
	static OpenClRayTracer* renderer;
	static btDiscreteDynamicsWorld* physics;
private:
	MultiInstance graphicsObject;
	btCollisionShape* physicsShape = nullptr;
};

