
#include "stdafx.h"
#include "GameServer.hpp"


int main()
{
	unsigned short serverPort = 6536;
	GameServer server(serverPort);

	while (!server.shouldExit()) {
		server.update();
	}

	return 0;
}