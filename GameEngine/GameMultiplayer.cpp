#include "stdafx.h"
#include "GameMultiplayer.hpp"
#include "NetworkDiscreteDynamicsWorld.hpp"
#include "Options.hpp"

#include <sstream>


GameMultiplayer::GameMultiplayer() : GameEngine(), serverAddress(options.serverAddress), serverPort(options.serverPort), playerName(options.playerName) {
	renderer->initializeAdvancedRender();

	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	physics = new NetworkDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	physics->setGravity(btVector3(0, -9.82f, 0));


	
	connectToServer();
}

void GameMultiplayer::connectToServer() {
	sf::Packet connectPackage;
	connectPackage << PacketType::CONNECTION_REQUEST;
	connectPackage << playerName;

	socket.setBlocking(true);
	socket.bind(sf::Socket::AnyPort, serverAddress);
	socket.send(connectPackage, serverAddress, serverPort);

	sf::Packet serverResponse;
	sf::IpAddress address;
	ushort port;
	do {
		socket.receive(serverResponse, address, port);
	} while (port != serverPort);
	PacketType packetType;
	serverResponse >> packetType;

	if (packetType != PacketType::INITIAL_GAMESTATE) {
		if (packetType == PacketType::CONNECTION_ERROR_RESPONSE)
			throw "Failed to connect to to server: " + packetType;
		else
			throw "Failed to connect to to server, invalid server response";
	}

	setupScene(serverResponse);
	socket.setBlocking(false);
}

void GameMultiplayer::setupScene(sf::Packet& gameScenePacket) {

	std::string sceneStr;

	gameScenePacket >> sceneStr;
	std::istringstream ss(sceneStr);

	GameBase::openScene(ss, nullptr, renderer);
	sf::Uint16 playerIndex;

	gameScenePacket >> playerIndex;

	player = getGameEntities()[playerIndex];
}

GameMultiplayer::~GameMultiplayer() {

}

void GameMultiplayer::initialize()
{
}

void GameMultiplayer::update()
{
	ushort port;
	sf::IpAddress address;
	sf::Packet gameStatePacket;
	while (socket.receive(gameStatePacket, address, port) == sf::Socket::Status::Done) {

		PacketType packetType;
		gameStatePacket >> packetType;

		switch (packetType)
		{
		case SERVER_TO_CLIENT_GAMESTATE:
			((NetworkDiscreteDynamicsWorld*)physics)->interpolateWorldState(gameStatePacket);
			break;
		case PacketType::CLIENT_TO_SERVER_UPDATE://Ignore update
			break;
		default:
			throw "Invalid server package";
			break;
		}
	}
	
	updateTime();
	input.readInput(deltaTime);
	for (auto entity : getGameEntities())
		entity->update(deltaTime);
	//GameEngine::update();


	sf::Packet playerInputPacket;
	playerInputPacket << PacketType::CLIENT_TO_SERVER_UPDATE;
	playerInputPacket << (sf::Uint16)player->getArrayIndex();
	playerInputPacket << input;
	socket.send(playerInputPacket, serverAddress, serverPort);
}
