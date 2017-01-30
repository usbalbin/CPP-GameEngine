#pragma once

#include <functional>

#include "bullet/btBulletDynamicsCommon.h"
#include "SFML\Network.hpp"

struct CollisionObjectData {
	btTransform transform;
	sf::Uint16 index;
};

sf::Packet& operator >> (sf::Packet& packet, CollisionObjectData& data);
sf::Packet& operator << (sf::Packet& packet, CollisionObjectData& data);

class NetworkDiscreteDynamicsWorld : public btDiscreteDynamicsWorld
{
public:
	NetworkDiscreteDynamicsWorld(btDispatcher * dispatcher, btBroadphaseInterface * pairCache, btConstraintSolver * constraintSolver, btCollisionConfiguration * collisionConfiguration);
	~NetworkDiscreteDynamicsWorld();

	btAlignedObjectArray<btRigidBody*>& getNonStaticRigidBodies();
	btCollisionObject* getCollisionObject(int worldIndex);

	void setWorldState(sf::Packet packet, std::function<void(btRigidBody*, const btTransform&)> p = [](btRigidBody* body, const btTransform& transform) { body->proceedToTransform(transform); });
	void interpolateWorldState(sf::Packet packet);

	void pushWorldState(sf::Packet packet);

	void getWorldState(sf::Packet& packet, unsigned tickCounter);

private:
	unsigned lastTickCounter = 0;
};

