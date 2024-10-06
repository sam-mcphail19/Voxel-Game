#include "voxelGame.hpp"

namespace g = voxel_game::graphics;

namespace voxel_game
{
	VoxelGame::VoxelGame()
	{
		m_window.setBackground(0.2, 0.2, 0.4);

		g::createTextureAtlas();
		g::loadTextureAtlas();

		m_player = new Player(glm::vec3(0, 45, 0));

		physics::Transform crosshairTransform(
			glm::vec3(-0.5, -0.5, 0),
			glm::mat4(1),
			glm::vec3(9.f/16, 1, 1) * glm::vec3(0.05)
		);

		m_crosshair = g::Quad::createQuad(crosshairTransform, g::loadTexture("res/texture/crosshair.png"));

		world::NoiseGenerator noiseGenerator(0L);
		world::WorldGenerator generator(noiseGenerator);
		m_world = new world::World(generator, &m_shader, *m_player);
		m_world->generate();
	}

	void VoxelGame::update()
	{
		const auto startTime = std::chrono::steady_clock::now();
		const auto msSinceLastTick = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - m_lastTickTime).count();

		m_shader.bind();

		// TODO: need to also handle cases where game is running too slow
        // can run multiple ticks or start skipping ticks
		if (msSinceLastTick > MS_BETWEEN_TICKS)
		{
			m_lastTickTime = startTime;

			updateGame();

			m_ticks++;
		}

		m_window.update();

		m_renderer.submitOrthoMesh(m_crosshair);

		for (world::Chunk* chunk : m_world->getVisibleChunks())
		{
			m_renderer.submitPerspMesh(chunk->getMesh());
		}

		m_renderer.render(&m_shader, m_player->getCamera(), &m_window);

		m_renderer.clearMeshes();

		m_shader.unbind();

		const auto endTime = std::chrono::steady_clock::now();

		const auto msSinceLastTpsLog = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - m_lastTpsLogTime).count();

		if (msSinceLastTpsLog > 1000)
		{
			log::info("TPS: " + std::to_string(m_ticks));
			m_lastTpsLogTime = startTime;
			m_ticks = 0;
		}

		const auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
		const auto fps = 1000000000.f / durationNs;
		m_minFps = std::min((float)m_minFps, fps);

		if (std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_lastFpsUpdateTime).count() > TIME_BETWEEN_FPS_COUNTER_UPDATES_NS)
		{
			m_window.setTitle(std::string("Voxel Game") + " FPS: " + voxel_game::utils::formatF(fps, 1) + "/" + voxel_game::utils::formatF(m_minFps, 1));
			m_minFps = LONG_MAX;
			m_lastFpsUpdateTime = endTime;
		}
	}

	void VoxelGame::updateGame()
	{
		m_world->update();
	}

	bool VoxelGame::shouldClose()
	{
		return glfwWindowShouldClose(m_window.getWindow());
	}
}