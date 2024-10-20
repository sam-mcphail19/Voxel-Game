#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "input/input.hpp"
#include "graphics/camera.hpp"
#include "util/mathUtils.hpp"
#include "world/chunk.hpp"
#include "constants.hpp"

namespace voxel_game
{
	struct PlayerControl
	{
		float pitch;
		float yaw;
		glm::vec3 movement;
		bool mouseOneDown;
		bool mouseOnePressed;
		bool mouseTwoDown;
		bool mouseTwoPressed;
	};

	class Player
	{
	private:
		glm::vec3 m_pos;
		graphics::Camera* m_camera;
		// collider
		float m_pitch = 0;
		float m_yaw = 180;
		double m_mouseX = 0;
		double m_mouseY = 0;
		bool m_isJumping = false;
		bool m_isCreativeMode = true;

		glm::vec3 calculateViewDir();

		void addPitch(float pitch);
		void addYaw(float yaw);

	public:
		Player(glm::vec3 pos);
		PlayerControl getInput();
		bool isAffectedByGravity();
		bool isAffectedByCollision();
		bool chunkIsVisible(world::Chunk* chunk);

		glm::vec3 getPos() const;
		glm::vec3 getVelocity();
		graphics::Camera* getCamera();

		void setPos(glm::vec3 pos);
		void setVelocity(glm::vec3 vel);
	};
}