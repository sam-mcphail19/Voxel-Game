#pragma once

#include <chrono>
#include <format>
#include <string>
#include <glm/vec3.hpp>
#include "graphics/mesh.hpp"
#include "graphics/quad.hpp"
#include "graphics/renderer.hpp"
#include "graphics/textureAtlas.hpp"
#include "util/log.hpp"
#include "util/stringUtils.hpp"
#include "world/world.hpp"
#include "player.hpp"

#define TIME_BETWEEN_FPS_COUNTER_UPDATES_NS 300000000
// 60 TPS i.e. 1000 / 60
#define MS_BETWEEN_TICKS 16.666666667

namespace voxel_game
{
	class VoxelGame
	{
	private:
		const std::string m_title = "Voxel Game";

		graphics::Renderer m_renderer = graphics::Renderer();
		graphics::Window m_window = graphics::Window(m_title, 1280, 720);
		graphics::Shader m_chunkShader = graphics::Shader("chunk.vs", "chunk.fs");
		graphics::Shader m_uiShader = graphics::Shader("ui.vs", "ui.fs");

		std::chrono::time_point<std::chrono::steady_clock> m_lastFpsUpdateTime;
		std::chrono::time_point<std::chrono::steady_clock> m_lastTickTime;
		std::chrono::time_point<std::chrono::steady_clock> m_lastTpsLogTime;
		int m_minFps = INT_MAX;
		int m_ticks = 0;

		Player *m_player;
		world::World *m_world;
		graphics::Quad *m_crosshair;

	public:
		VoxelGame();
		void update();
		bool shouldClose();

	private:
		void updateGame();
		void draw();
	};
}