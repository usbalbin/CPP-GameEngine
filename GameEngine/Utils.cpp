#include "stdafx.h"

#include "Utils.hpp"
#include "GLFW\glfw3.h"
#include "OpenClTesting\Utils.hpp"


btVector3 toVector3(glm::vec3 vector) {
	return btVector3(vector.x, vector.y, vector.z);
}

glm::vec3 toVec3(btVector3 vector) {
	return glm::vec3(vector.x(), vector.y(), vector.z());
}

glm::vec3 toVec3(glm::mat4 matrix)
{
	return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}

btTransform toTransform(glm::mat4 matrix) {
	btTransform transform;
	transform.setFromOpenGLMatrix((float*)glm::value_ptr(glm::transpose(matrix)));
	return transform;
}

btTransform toTransform(glm::vec3& position, float yaw, float pitch, float roll)
{
	return btTransform(btQuaternion(yaw, pitch, roll), toVector3(position));
}

glm::mat4 toMatrix(btTransform transform) {
	float values[16];
	transform.getOpenGLMatrix(values);
	return glm::transpose(glm::make_mat4(values));
}

glm::mat4 toMatrix(glm::vec3 position)
{
	glm::mat4 result;
	result[0][3] = position.x;
	result[1][3] = position.y;
	result[2][3] = position.z;
	return result;
}

bool readGamingWheel(float* wheel, float* throttle, float* brake) {
	int present = glfwJoystickPresent(0);

	if (!present) {
		*wheel = 0;
		*throttle = 0;
		*brake = 0;
		return false;
	}

	int count;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

	*wheel = axes[0];
	*throttle = mapToRange(axes[1], +1.0f, -1.0f, 0.0f, 1.0f);
	*brake = mapToRange(axes[2], +1.0f, -1.0f, 0.0f, 1.0f);
	return true;
}

float angleDiff(float a, float b)
{
	float diff = btNormalizeAngle(a) - btNormalizeAngle(b);
	return btNormalizeAngle(diff);
}
