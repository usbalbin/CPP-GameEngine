#include "stdafx.h"
#include "NetworkDiscreteDynamicsWorld.hpp"
#include "Containers.hpp"


NetworkDiscreteDynamicsWorld::NetworkDiscreteDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, btConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration) : btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
{
}


NetworkDiscreteDynamicsWorld::~NetworkDiscreteDynamicsWorld()
{
}

btAlignedObjectArray<btRigidBody*>& NetworkDiscreteDynamicsWorld::getNonStaticRigidBodies()
{
	return m_nonStaticRigidBodies;
}

btCollisionObject* NetworkDiscreteDynamicsWorld::getCollisionObject(int worldIndex) {
	return m_collisionObjects[worldIndex];
}

void NetworkDiscreteDynamicsWorld::setWorldState(sf::Packet packet, std::function<void(btRigidBody*, const btTransform&)> p) {
	sf::Uint16 objectCount;
	sf::Uint16 tickCounter;
	packet >> objectCount;
	packet >> tickCounter;

	if (tickCounter <= lastTickCounter) {
		//Old package
		return;
	}
	lastTickCounter = tickCounter;

	if (objectCount != m_nonStaticRigidBodies.size())
		 throw "Game not in sync with server!";

	for (int i = 0; i < objectCount; i++) {
		CollisionObjectData data;
		packet >> data;

		auto body = (btRigidBody*)getCollisionObject(data.index);

		
		p(body, data.transform);
	}
}

void NetworkDiscreteDynamicsWorld::interpolateWorldState(sf::Packet packet)
{
	float blend = 0.5f;

	auto p = [blend](btRigidBody* body, const btTransform& transform) {
		const btTransform& currentTransform = body->getWorldTransform();
		
		btTransform newTransform;
		newTransform.setRotation(
			currentTransform.getRotation().slerp(transform.getRotation(), blend)
		);
		newTransform.setOrigin(
			currentTransform.getOrigin().lerp(transform.getOrigin(), blend)
		);
		body->proceedToTransform(newTransform);
	};

	setWorldState(packet, p);
}

void NetworkDiscreteDynamicsWorld::pushWorldState(sf::Packet packet)
{
	float blend = 0.5f;

	auto p = [blend](btRigidBody* body, const btTransform& transform) {
		btVector3 rotationalFactor = 1.0f * body->getLocalInertia();
		float linearFactor = 1.0f / body->getInvMass();
		
		const btTransform& currentTransform = body->getWorldTransform();

		btVector3 currentRot;
		currentTransform.getBasis().getEulerYPR((btScalar&)currentRot.x(), (btScalar&)currentRot.y(), (btScalar&)currentRot.z());
		btVector3 serverRot;
		transform.getBasis().getEulerYPR((btScalar&)currentRot.x(), (btScalar&)currentRot.y(), (btScalar&)currentRot.z());
		
		btVector3 currentPos = currentTransform.getOrigin();
		btVector3 serverPos = transform.getOrigin();

		btVector3 deltaRot = serverRot - currentRot;
		deltaRot.setX(btNormalizeAngle(deltaRot.x()));
		deltaRot.setY(btNormalizeAngle(deltaRot.y()));
		deltaRot.setZ(btNormalizeAngle(deltaRot.z()));

		body->applyTorque(deltaRot * rotationalFactor);
		body->applyCentralForce((serverPos - currentPos) * linearFactor);
	};

	setWorldState(packet, p);
}

void NetworkDiscreteDynamicsWorld::getWorldState(sf::Packet & packet, unsigned tickCounter) {
	unsigned char objectCount = m_nonStaticRigidBodies.size();
	packet << objectCount;
	packet << tickCounter;


	if (objectCount != m_nonStaticRigidBodies.size())
		throw "Game not in sync with server!";

	for (int i = 0; i < objectCount; i++) {
		CollisionObjectData data;
		

		auto body = (btRigidBody*)m_nonStaticRigidBodies[i];

		//TODO: Make sure this is the way to go, regarding correction of body transforms
		data.transform = body->getWorldTransform();
		data.index = body->getWorldArrayIndex();

		packet << data;
	}
}

sf::Packet & operator >> (sf::Packet & packet, CollisionObjectData & data)
{
	packet >> data.transform;
	packet >> data.index;
	return packet;
}

sf::Packet & operator<<(sf::Packet & packet, CollisionObjectData & data)
{
	return packet << data.transform << data.index;
}
