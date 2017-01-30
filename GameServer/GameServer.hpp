#pragma once

#include "GameEngine\Containers.hpp"

#include "SFML\Network.hpp"
#include "bullet\LinearMath/btTransform.h"
#include "bullet\LinearMath/btQuaternion.h"
#include "bullet\btBulletDynamicsCommon.h"

#include <tuple>
#include <unordered_map>
#include <string>


#include "GameEngine\GameBase.hpp"


struct Hasher {
	unsigned long long operator()(
		const std::pair<sf::IpAddress, ushort> p
		) const;
};

struct Equal {
	unsigned long long operator()(
		const std::pair<sf::IpAddress, ushort> left,
		const std::pair<sf::IpAddress, ushort> right
		) const;
};



class GameServer : public GameBase
{
public:
	GameServer(unsigned short serverPort);
	~GameServer();
	void initialize() override;
	void update() override;
private:
	void addClient(sf::IpAddress clientAddress, ushort clientPort, sf::Packet packet);
	void updateClient(sf::IpAddress clientAddress, ushort clientPort, sf::Packet packet);
	void broadcastToClients();
	void dropTimeoutedClients();
	void sendGameState(sf::Packet packet);
	void sendInitialScene(sf::IpAddress clientAddress, ushort clientPort);

	std::string initialScene = "";
	TimePoint lastBroadcast;
	Duration refreshRate;
	sf::Uint16 tickCounter = 1337;
	int maxPlayerCount = 16;
	ushort serverPort;
	sf::UdpSocket socket;
	std::unordered_map<std::pair<sf::IpAddress, ushort>, Client, Hasher, Equal> clients;
};

