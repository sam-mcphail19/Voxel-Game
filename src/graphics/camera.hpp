#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace voxel_game::graphics
{
	class Camera
	{
	private:
		glm::vec3 m_pos;
		float m_pitch;
		float m_yaw;

	public:
		Camera(glm::vec3 pos, float pitch, float yaw);
		glm::vec3 getPos();
		glm::mat4 viewMatrix();
		glm::vec3 viewDir();

		void setYaw(float yaw);
		void setPitch(float pitch);
		void setPos(glm::vec3 pos);
	};

}
