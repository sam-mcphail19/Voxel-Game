#include "camera.hpp"

namespace voxel_game::graphics
{
	Camera::Camera(glm::vec3 pos, float pitch, float yaw)
	{
		m_pos = pos;
		m_pitch = pitch;
		m_yaw = yaw;
	}

	glm::vec3 Camera::getPos()
	{
		return m_pos;
	}

	glm::mat4 Camera::viewMatrix()
	{
		glm::mat4 mat = glm::rotate(glm::mat4(1.f), glm::radians(m_pitch), glm::vec3(1.0, 0.0, 0.0));
		mat = glm::rotate(mat, glm::radians(m_yaw), glm::vec3(0.0, 1.0, 0.0));
		return glm::translate(mat, m_pos * -1.f);
	}

	glm::vec3 Camera::viewDir()
	{
		float pitchRad = glm::radians(m_pitch);
		float yawRad = glm::radians(m_yaw);

		glm::vec3 result = glm::vec3(glm::sin(yawRad) * glm::cos(pitchRad),
									 -glm::sin(pitchRad),
									 -glm::cos(yawRad) * glm::cos(pitchRad));
		return glm::normalize(result);
	}

	void Camera::setPitch(float pitch)
	{
		m_pitch = pitch;
	}

	void Camera::setYaw(float yaw)
	{
		m_yaw = yaw;
	}

	void Camera::setPos(glm::vec3 pos)
	{
		m_pos = pos;
	}
}