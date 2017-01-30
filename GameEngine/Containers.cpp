#include "stdafx.h"
#include "Containers.hpp"


sf::Packet& operator >> (sf::Packet& packet, PacketType& packetType) {
	sf::Int8& data = (sf::Int8&)packetType;
	
	return packet >> data;
}

sf::Packet & operator >> (sf::Packet & packet, btTransform & transform) {
	btQuaternion orientation;
	btVector3 position;
	packet >> orientation >> position;
	transform.setRotation(orientation);
	transform.setOrigin(position);
	return packet;
}

sf::Packet & operator >> (sf::Packet & packet, btQuaternion & orientation) {
	float x, y, z, w;
	packet >> x >> y >> z >> w;
	orientation.setX(x);
	orientation.setY(y);
	orientation.setZ(z);
	orientation.setW(w);
	return packet;
}

sf::Packet & operator >> (sf::Packet & packet, btVector3 & position) {
	float x, y, z;
	packet >> x >> y >> z;
	position.setX(x);
	position.setY(y);
	position.setZ(z);
	position.setW(0);
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const PacketType packetType) {
	return packet << (sf::Int8&)packetType;
}

sf::Packet & operator << (sf::Packet & packet, const btTransform & transform) {
	return packet << transform.getRotation() << transform.getOrigin();
}

sf::Packet & operator << (sf::Packet & packet, const btQuaternion & orientation) {
	return packet << orientation.getX() << orientation.getY() << orientation.getZ() << orientation.getW();
}

sf::Packet & operator << (sf::Packet & packet, const btVector3 & position) {
	return packet << position.getX() << position.getY() << position.getZ();
}

TimePoint now() {
	return std::chrono::high_resolution_clock::now();
}
