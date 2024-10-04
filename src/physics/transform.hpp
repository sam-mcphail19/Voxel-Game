#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace voxel_game::physics
{
	class Transform
	{
	private:
		glm::vec3 m_translation;
		glm::mat4 m_rotation;
		glm::vec3 m_scale;

	public:
		Transform();
		Transform(glm::vec3 translation, glm::mat4 rotation, glm::vec3 scale);
		glm::mat4 modelMat();

		void setTranslation(glm::vec3 translation);
	};
}