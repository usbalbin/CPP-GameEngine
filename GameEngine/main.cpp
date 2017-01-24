#include "stdafx.h"

#include "GameEngine.hpp"
#include "Box.hpp"
#include "Sphere.hpp"






int main() {
	GameEngine game(640, 480);
	game.initialize();
	
	while (!game.shouldExit()) {
		game.update();
		game.draw();
	}


}