#include "stdafx.h"

#include "GameEngine/GameEngine.hpp"
#include "GameEngine/GameMultiplayer.hpp"

void playMulti() {
	GameMultiplayer game("localhost", 6536, 1024, 768);
	game.initialize();

	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}
}

void playSingle() {
	GameEngine game(1024, 768);
	game.initialize();

	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}
}

int main() {
	playMulti();
}