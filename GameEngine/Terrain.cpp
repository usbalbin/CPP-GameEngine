#include "stdafx.h"
#include "Terrain.hpp"

#include "TerrainShape.hpp"

Terrain::Terrain(OpenClRayTracer * renderer, btDiscreteDynamicsWorld * physics) : 
	Entity(renderer, physics) {
	parts.push_back(
		new TerrainShape(renderer, physics)
	);


}

Terrain::~Terrain()
{
}

/*
void Terrain::draw() {
	int i = 0; 
	system("echo hoj");
	Entity::draw();
}*/