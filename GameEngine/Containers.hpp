#pragma once

#include "SFML\Network.hpp"
#include "bullet\LinearMath\btTransform.h"

#include <chrono>


typedef unsigned short ushort;
typedef std::chrono::duration<float> Duration;
typedef std::chrono::time_point<std::chrono::high_resolution_clock, Duration> TimePoint;


TimePoint now();



struct Client {
	Client(sf::IpAddress address, ushort port, std::string name) : address(address), port(port), name(name), lastMessage(now()) {}
	Client() {};
	std::string toString() const { return name + "@" + address.toString() + ":" + std::to_string(port); }
	sf::IpAddress address;
	ushort port;
	std::string name;
	TimePoint lastMessage;

	btTransform transform;
};

enum PacketType : sf::Int8 {
	CONNECTION_REQUEST = 0,
	CONNECTION_ERROR_RESPONSE = 1,
	INITIAL_GAMESTATE = 2,
	CLIENT_TO_SERVER_UPDATE = 3,
	SERVER_TO_CLIENT_GAMESTATE = 4
};

enum ResponseStatus : sf::Int8 {
	SUCCESS = 0,
	SERVER_FULL = 1,
	UNKNOWN_ERROR = 2
};



sf::Packet& operator >> (sf::Packet& packet, PacketType& packetType);
sf::Packet& operator >> (sf::Packet& packet, btTransform& transform);
sf::Packet& operator >> (sf::Packet& packet, btQuaternion& orientation);
sf::Packet& operator >> (sf::Packet & packet, btVector3 & position);

sf::Packet& operator << (sf::Packet& packet, const PacketType packetType);
sf::Packet& operator << (sf::Packet& packet, const btTransform& transform);
sf::Packet& operator << (sf::Packet& packet, const btQuaternion& orientation);
sf::Packet& operator << (sf::Packet & packet, const btVector3 & position);