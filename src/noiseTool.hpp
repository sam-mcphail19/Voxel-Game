#pragma once

#include <iostream>
#include <array>
#include <atomic>
#include <map>
#include <thread>
#include "graphics/renderer.hpp"
#include "graphics/window.hpp"
#include "world/biomeBasedWorldGenerator.hpp"
#include "world/block.hpp"
#include "world/splineBasedWorldGenerator.hpp"
#include "util/log.hpp"
#include "vendor/lodepng.h"
#include "vendor/stb_image.h"

#define NOISE_TEX_SIZE 1024

namespace voxel_game
{
	struct BlockPixel
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	class NoiseTool
	{
	private:
		enum class NoiseView
		{
			Surface,
			Continentalness,
			Erosion,
			Temperature,
			Humidity,
			Ridges,
			Coasts,
			Slope,
			PlateauCliffs,
			Formations,
			Rivers,
			TerrainTypes,
			Biomes,
			Height,
			Count
		};

		const std::string m_title = "Voxel Game - Noise Tool";
		long m_seed = 0;
		NoiseView m_view = NoiseView::Surface;

		graphics::Window m_window = graphics::Window(m_title, NOISE_TEX_SIZE, NOISE_TEX_SIZE);
		graphics::Renderer m_renderer = graphics::Renderer(m_window);
		graphics::Shader m_shader = graphics::Shader("ui.vs", "ui.fs");
		graphics::Camera* m_camera = new graphics::Camera(glm::vec3(0, 0, 0), 0, 180);

		physics::Transform* m_transform;
		graphics::Texture* m_texture = nullptr;
		graphics::Quad* m_tex = nullptr;
		std::vector<world::TerrainDebugSample> m_samples;

	public:
		NoiseTool();
		void generate(long seed);
		void cycleView();

		void draw();
		bool shouldClose();

	private:
		void sampleSeed(long seed);
		void renderCurrentView();
	};

	inline constexpr unsigned char operator "" _uchar(unsigned long long arg) noexcept
	{
		return static_cast<unsigned char>(arg);
	}
}
