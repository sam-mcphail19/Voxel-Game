#include "transform.hpp"

namespace voxel_game::physics {
	Transform::Transform(glm::vec3 translation, glm::mat4 rotation, glm::vec3 scale)
	{
		m_translation = translation;
		m_rotation = rotation;
		m_scale = scale;
	}

	Transform::Transform()
	{
		m_translation = glm::vec3(0.f);
		m_rotation = glm::mat4(1.f);
		m_scale = glm::vec3(1.f);
	}

	glm::mat4 Transform::modelMat()
	{
		glm::mat4 mat = glm::scale(glm::mat4(1.f), m_scale);
		mat *= m_rotation;
		return glm::translate(mat, m_translation);
	}

	void Transform::setTranslation(glm::vec3 translation)
	{
		m_translation = translation;
	}
}