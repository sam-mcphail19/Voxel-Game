#pragma once

#include <iostream>
#include <map>
#include "graphics/renderer.hpp"
#include "graphics/window.hpp"
#include "world/block.hpp"
#include "world/worldGenerator.hpp"
#include "util/log.hpp"
#include "vendor/lodepng.h"
#include "vendor/stb_image.h"

#define NOISE_TEX_PATH "noise_tool/texture.png"
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
		const std::string m_title = "Voxel Game - Noise Tool";

		graphics::Window m_window = graphics::Window(m_title, NOISE_TEX_SIZE, NOISE_TEX_SIZE);
		graphics::Renderer m_renderer = graphics::Renderer(m_window);
		graphics::Shader m_shader = graphics::Shader("ui.vs", "ui.fs");
		graphics::Camera* m_camera = new graphics::Camera(glm::vec3(0, 0, 0), 0, 180);

		physics::Transform* m_transform;
		graphics::Quad* m_tex;

	public:
		NoiseTool();
		void generate(long seed);

		void draw();
		bool shouldClose();

	private:
		void createNoiseTexture(long seed);
		void createNoiseTextureGreyScale(long seed);
	};

	inline constexpr unsigned char operator "" _uchar(unsigned long long arg) noexcept
	{
		return static_cast<unsigned char>(arg);
	}
}