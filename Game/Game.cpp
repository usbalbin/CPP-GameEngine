#include "stdafx.h"

#include "GameEngine/Options.hpp"

#include "GameEngine/GameEngine.hpp"
#include "GameEngine/GameMultiplayer.hpp"


#include "OpenClTesting\ClReadBuffer.hpp"


void playMulti() {
	GameMultiplayer game;
	game.initialize();

	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}
}

void playSingle() {
	GameEngine game;
	game.initialize();

	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}
}


int main() {
	playSingle();
}