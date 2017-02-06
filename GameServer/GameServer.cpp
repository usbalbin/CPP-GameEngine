// GameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GameServer.hpp"
#include <iostream>
#include <fstream>
#include "GameEngine\DebugTools.hpp"
#include "GameEngine\NetworkDiscreteDynamicsWorld.hpp"


using namespace std::chrono_literals;



unsigned long long Hasher::operator()(const std::pair<sf::IpAddress, ushort> p) const {
	return ((unsigned long long)p.first.toInteger() << 32) | p.second;
}

unsigned long long Equal::operator()(const std::pair<sf::IpAddress, ushort> left, const std::pair<sf::IpAddress, ushort> right) const {
	return (((unsigned long long)left.first.toInteger() << 32) | left.second) ==
		(((unsigned long long)right.first.toInteger() << 32) | right.second);
}


GameServer::GameServer(unsigned short serverPort) : serverPort(serverPort){
	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	physics = new NetworkDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	physics->setGravity(btVector3(0, -9.82f, 0));

	if (socket.bind(serverPort) == sf::Socket::Status::Error) {
		std::cout << "Server already running?" << std::endl;
		system("pause");
		exit(4);
	}
	socket.setBlocking(false);

	std::cout << "Server started at port: " << serverPort << std::endl;

	this->lastBroadcast = now();

	std::ifstream sceneFile("content/scene.scene");
	openScene(sceneFile);
	sceneFile.clear();
	sceneFile.seekg(0, sceneFile.beg);
	std::string line;
	while (getline(sceneFile, line))
		initialScene += line + '\n';
}


GameServer::~GameServer() {
	auto broadphase = physics->getBroadphase();
	auto dispatcher = physics->getDispatcher();
	auto solver = physics->getConstraintSolver();


	delete physics;
	delete solver;
	delete dispatcher;
	//delete collisionConfiguration;
	delete broadphase;
}

void GameServer::initialize()
{
}

void GameServer::update() {

	sf::Packet packet;
	sf::IpAddress clientAddress;
	ushort clientPort;

	//Look for client updates
	for (int i = 0; i < maxPlayerCount && socket.receive(packet, clientAddress, clientPort) == sf::Socket::Done; i++) {
		if (clients.find({ clientAddress, clientPort }) == clients.end())
			addClient(clientAddress, clientPort, packet);
		else
			updateClient(clientAddress, clientPort, packet);
	}

	Duration deltaTime (now() - lastBroadcast);
	if (deltaTime >= refreshRate) {
		GameBase::updateTime();
		GameBase::update();

		broadcastToClients();
		lastBroadcast += deltaTime;
	}
}

void GameServer::addClient(sf::IpAddress clientAddress, ushort clientPort, sf::Packet packet){
	debugPrintln("Client connecting");

	PacketType packetType;
	packet >> packetType;

	sf::Packet responsePackage;
	ResponseStatus responseStatus;
	if (packetType == PacketType::CONNECTION_REQUEST) {
		if (clients.size() < maxPlayerCount) {
			std::string playerName;

			packet >> playerName;

			clients[{clientAddress, clientPort}] = Client(clientAddress, clientPort, playerName);
			responseStatus = ResponseStatus::SUCCESS;


			std::cout << "Player " << clients[{clientAddress, clientPort}].toString() << " joined" << std::endl;
		}
		else {
			responsePackage << PacketType::CONNECTION_ERROR_RESPONSE << ResponseStatus::SERVER_FULL;
			socket.send(responsePackage, clientAddress, clientPort);
		}
		
	}else {

		std::cout << "Warning!" << std::endl;
		std::cout << "Invalid package " << (int)packetType << " from " << clientAddress.toString() << "@" << clientPort << std::endl;
		responsePackage << PacketType::CONNECTION_ERROR_RESPONSE << ResponseStatus::UNKNOWN_ERROR;
		socket.send(responsePackage, clientAddress, clientPort);
	}

	sendInitialScene(clientAddress, clientPort);
}

void GameServer::updateClient(sf::IpAddress clientAddress, ushort clientPort, sf::Packet packet) {
	PacketType packetType;
	packet >> packetType;

	sf::Uint16 entityId;
	Input playerInput;
	Client& client = clients[{clientAddress, clientPort}];

	auto currentTime = now();
	const float deltaTime = (currentTime - client.lastMessage).count();

	switch (packetType)
	{
	case PacketType::CLIENT_TO_SERVER_UPDATE:
		packet >> entityId;
		packet >> playerInput;
		
		getGameEntities()[entityId]->handleInput(playerInput, deltaTime);
		client.lastMessage = currentTime;
		break;
	default:
		std::cout << "Warning!" << std::endl;
		std::cout << "Invalid package from " << clients[{clientAddress, clientPort}].toString() << std::endl;
	}

}

void GameServer::broadcastToClients() {
	dropTimeoutedClients();
	
	NetworkDiscreteDynamicsWorld* physics = (NetworkDiscreteDynamicsWorld*)this->physics;
	sf::Uint16 nonStaticRigidBodyCount = physics->getNonStaticRigidBodies().size();
	sf::Packet gameState;
	gameState << PacketType::SERVER_TO_CLIENT_GAMESTATE;
	gameState << nonStaticRigidBodyCount;
	gameState << tickCounter++;

	for(int i = 0; i < physics->getNonStaticRigidBodies().size(); i++) {
		CollisionObjectData data;
		data.transform = physics->getNonStaticRigidBodies()[i]->getWorldTransform();
		data.index = physics->getNonStaticRigidBodies()[i]->getWorldArrayIndex();

		gameState << data;
	}

	
	sendGameState(gameState);
}

void GameServer::dropTimeoutedClients() {
	auto timeOut = 10s;
	auto currentTime = now();

	for (auto& it = clients.begin(); it != clients.end();) {
		if (currentTime - it->second.lastMessage >= timeOut) {
			std::cout << "Player " << it->second.toString() << " timed out." << std::endl;
			it = clients.erase(it);
		}else
			it++;
	}
}

void GameServer::sendGameState(sf::Packet packet){
	for (auto& p : clients) {
		const auto& client = p.second;
		socket.send(packet, client.address, client.port);
	}
}

void GameServer::sendInitialScene(sf::IpAddress clientAddress, ushort clientPort)
{
	//TODO: find better way
	static sf::Uint16 playerEntityIndex = 0;

	sf::Packet packet;
	packet << PacketType::INITIAL_GAMESTATE;
	packet << initialScene;
	packet << playerEntityIndex++;
	socket.send(packet, clientAddress, clientPort);
}

