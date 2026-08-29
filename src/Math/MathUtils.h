#pragma once
#include <glm/glm.hpp>


namespace MathUtils
{
	glm::quat EularToQuaternion(const glm::vec3& euler);
	glm::vec3 GenerateUpVector(const glm::vec3& forward);
}