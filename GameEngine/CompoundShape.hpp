#pragma once

#include "CompoundShapeChild.hpp"

#include <vector>

class CompoundShape : public Shape
{
public:
	CompoundShape(ClRayTracer * renderer, btDiscreteDynamicsWorld * physics);
	~CompoundShape();

	void addChild(CompoundShapeChild * child);
	void finalize(glm::vec3 position, std::vector<float> masses, float yaw, float pitch, float roll);
	

	void draw();
	void update(float deltaTime);
private:
	btCompoundShape* physicsShape;
	std::vector<CompoundShapeChild*> childShapes;
};

