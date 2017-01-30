#pragma once

#include "glm\vec3.hpp"
#include "glm\mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "LinearMath\btTransform.h"
#include "LinearMath\btTransformUtil.h"
#include "SFML/System/Vector3.hpp"

#include <vector>

const float PI_HALF = 1.57079632679;
const float PI = 3.14159265359;

btVector3 toVector3(glm::vec3 vector);
sf::Vector3f toSfVector3(glm::vec3 vector);
sf::Vector3f toSfVector3(glm::vec4 vector);
sf::Vector3f toSfVector3(glm::mat4 matrix);


glm::vec3 toVec3(btVector3 vector);
glm::vec3 toVec3(glm::mat4 matrix);

btTransform toTransform(glm::mat4 matrix);
btTransform toTransform(glm::vec3& position, float yaw = 0, float pitch = 0, float roll = 0);

glm::mat4 toMatrix(btTransform transform);
glm::mat4 toMatrix(glm::vec3 position);

float angleDiff(float a, float b);

template<typename T>
bool contains(std::vector<T> data, T needle) {
	for (auto& elem : data)
		if (elem == needle)
			return true;
	return false;
}