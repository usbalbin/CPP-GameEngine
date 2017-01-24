#include "stdafx.h"

#include "GameEngine.hpp"
#include "Box.hpp"
#include "Sphere.hpp"



int main() {
	GameEngine game(1024, 768);
	game.initialize();
	
	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}


}