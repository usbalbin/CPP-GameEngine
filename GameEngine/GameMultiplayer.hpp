#pragma once

#include "GameEngine.hpp"

#include "Containers.hpp"


class GameMultiplayer :
	public GameEngine
{
public:
	GameMultiplayer(sf::IpAddress, ushort port, int width, int height);
	~GameMultiplayer();
	void initialize() override;
	void update() override;

private:
	void connectToServer();
	void setupScene(sf::Packet& gameScenePacket);

	sf::IpAddress serverAddress;
	ushort serverPort;
	sf::UdpSocket socket;
	std::string playerName = "Kamel-Hubert";
};

