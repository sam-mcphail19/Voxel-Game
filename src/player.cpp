#include "player.hpp"

namespace voxel_game
{
	const glm::vec3 up(0, 1, 0);

	const glm::vec3 size(0.9, 1.8, 0.9);
	const glm::vec3 cameraPosDiff(size.x / 2, 2, size.z / 2);

	Player::Player(glm::vec3 pos)
		: m_pos(pos), m_camera(new graphics::Camera(pos + cameraPosDiff, m_pitch, m_yaw))
	{
		input::getMousePos(m_mouseX, m_mouseY);
	}

	PlayerControl Player::getInput()
	{
		double oldX = m_mouseX;
		double oldY = m_mouseY;
		input::getMousePos(m_mouseX, m_mouseY);

		bool mouseOneDown = input::isButtonDown(GLFW_MOUSE_BUTTON_1);
		bool mouseOnePressed = input::isButtonPressed(GLFW_MOUSE_BUTTON_1);
		bool mouseTwoDown = input::isButtonDown(GLFW_MOUSE_BUTTON_2);
		bool mouseTwoPressed = input::isButtonPressed(GLFW_MOUSE_BUTTON_2);

		float pitch = (m_mouseY - oldY) * MOUSE_SENS;
		float yaw = (m_mouseX - oldX) * MOUSE_SENS;

		glm::vec3 viewDir = calculateViewDir();
		glm::vec3 strafeDir = glm::cross(viewDir, up);

		glm::vec3 movement = glm::vec3(0);
		float moveSpeed = m_isCreativeMode ? CREATIVE_MOVEMENT_SPEED : MOVEMENT_SPEED;
		if (input::isKeyPressed(GLFW_KEY_W))
		{
			movement += viewDir * moveSpeed;
		}
		if (input::isKeyPressed(GLFW_KEY_A))
		{
			movement += strafeDir * -moveSpeed;
		}
		if (input::isKeyPressed(GLFW_KEY_S))
		{
			movement += viewDir * -moveSpeed;
		}
		if (input::isKeyPressed(GLFW_KEY_D))
		{
			movement += strafeDir * moveSpeed;
		}

		if (m_isCreativeMode)
		{
			if (input::isKeyPressed(GLFW_KEY_SPACE))
			{
				movement.y += JUMP_SPEED;
			}
			if (input::isKeyPressed(GLFW_KEY_LEFT_CONTROL))
			{
				movement.y -= JUMP_SPEED;
			}
		}
		else
		{
			if (input::isKeyPressed(GLFW_KEY_SPACE) && !m_isJumping)
			{
				movement.y = JUMP_SPEED;
				m_isJumping = true;
			}
		}

		addPitch(pitch);
		addYaw(yaw);

		m_camera->setPos(m_pos);

		return { pitch, yaw, movement, mouseOneDown, mouseOnePressed, mouseTwoDown, mouseTwoPressed };
	}

	bool Player::isAffectedByGravity()
	{
		return !m_isCreativeMode && m_isJumping;
	}

	// TODO: Frustum culling
	bool Player::chunkIsVisible(world::Chunk* chunk)
	{
		world::BlockPos chunkOrigin = chunk->getOrigin();
		int chunkCenterX = chunkOrigin.x + CHUNK_SIZE / 2;
		int chunkCenterZ = chunkOrigin.z + CHUNK_SIZE / 2;

		if (std::abs(chunkCenterX - m_pos.x) > CHUNK_RENDER_DISTANCE_IN_BLOCKS ||
			std::abs(chunkCenterZ - m_pos.z) > CHUNK_RENDER_DISTANCE_IN_BLOCKS) {
			return false;
		}
		int x = m_pos.x - chunkCenterX;
		int z = m_pos.z - chunkCenterZ;
		return std::sqrt(x * x + z * z) < CHUNK_RENDER_DISTANCE_IN_BLOCKS;
	}

	glm::vec3 Player::getPos() const
	{
		return m_pos;
	}

	graphics::Camera* Player::getCamera()
	{
		return m_camera;
	}

	void Player::setPos(glm::vec3 pos)
	{
		m_pos = pos;
		m_camera->setPos(m_pos);
	}

	glm::vec3 Player::calculateViewDir()
	{
		float yawRad = glm::radians(m_yaw);

		return glm::normalize(glm::vec3(glm::sin(yawRad), 0, -glm::cos(yawRad)));
	}

	void Player::addPitch(float pitch)
	{
		m_pitch += pitch;

		m_pitch = utils::max(m_pitch, -90.f);
		m_pitch = utils::min(m_pitch, 90.f);

		m_camera->setPitch(m_pitch);
	}

	void Player::addYaw(float yaw)
	{
		m_yaw += yaw;

		if (m_yaw > 360)
		{
			m_yaw -= 360;
		}
		else if (m_yaw < 0)
		{
			m_yaw += 360;
		}

		m_camera->setYaw(m_yaw);
	}
}