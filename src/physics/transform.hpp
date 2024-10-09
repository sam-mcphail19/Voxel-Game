#pragma once

#include <string>
#include <sstream>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace voxel_game::physics
{
	class Transform
	{
	public:
		glm::vec3 m_translation;
		glm::mat4 m_rotation;
		glm::vec3 m_scale;

		Transform();
		Transform(glm::vec3 translation, glm::mat4 rotation, glm::vec3 scale);
		glm::mat4 modelMat();

		std::string toString();
	};
}

