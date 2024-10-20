#include "noiseTool.hpp"

namespace voxel_game
{
	const std::unordered_map<world::BlockTypeId, BlockPixel> blockColors = {
			{world::BlockTypeId::WATER, BlockPixel{30_uchar, 30_uchar, 255_uchar}},
			{world::BlockTypeId::STONE, BlockPixel{200_uchar, 200_uchar, 200_uchar}},
			{world::BlockTypeId::DIRT, BlockPixel{170_uchar, 40_uchar, 40_uchar}},
			{world::BlockTypeId::GRASS, BlockPixel{30_uchar, 220_uchar, 100_uchar}},
			{world::BlockTypeId::SAND, BlockPixel{200_uchar, 180_uchar, 130_uchar}},
	};

	NoiseTool::NoiseTool()
	{
		m_window.setBackground(0.05, 0.05, 0.05);

		physics::Transform* transform = new physics::Transform(
			glm::vec3(-0.5, -0.5, 0),
			glm::mat4(1),
			glm::vec3(1, 1, 1)
		);

		createNoiseTexture();

		m_tex = g::Quad::createQuad(transform, g::loadTexture(NOISE_TEX_PATH));
	}

	void NoiseTool::createNoiseTexture()
	{
		std::vector<unsigned char> result(NOISE_TEX_SIZE * NOISE_TEX_SIZE * 4, 255);
		world::BlockTypeId* blocks = new world::BlockTypeId[NOISE_TEX_SIZE * NOISE_TEX_SIZE];

		unsigned char alpha = static_cast<unsigned char>(255.f);

		for (int x = 0; x < NOISE_TEX_SIZE; ++x)
		{
			for (int y = 0; y < NOISE_TEX_SIZE; ++y)
			{
				int surfaceHeight = std::max(m_worldGenerator.getHeight(x, y), WATER_HEIGHT);

				world::BlockTypeId block = m_worldGenerator.getBlockType(world::BlockPos{ x, surfaceHeight, y });
				BlockPixel blockColor = blockColors.at(block);

				int resultIndex = (y * NOISE_TEX_SIZE + x) * 4;
				result[resultIndex] = blockColor.r;
				result[resultIndex + 1] = blockColor.g;
				result[resultIndex + 2] = blockColor.b;
				result[resultIndex + 3] = alpha;
			}
		}

		unsigned error = lodepng::encode(NOISE_TEX_PATH, result, NOISE_TEX_SIZE, NOISE_TEX_SIZE);
		if (error)
		{
			std::cout << "Error " << error << ": " << lodepng_error_text(error) << std::endl;
		}
	}

	void NoiseTool::createNoiseTextureGreyScale()
	{
		std::vector<unsigned char> result(NOISE_TEX_SIZE * NOISE_TEX_SIZE * 4, 255);
		int* heights = new int[NOISE_TEX_SIZE * NOISE_TEX_SIZE];

		int maxHeight = 0;

		unsigned char alpha = static_cast<unsigned char>(255.f);

		for (int x = 0; x < NOISE_TEX_SIZE; ++x)
		{
			for (int y = 0; y < NOISE_TEX_SIZE; ++y)
			{
				int height = m_worldGenerator.getHeight(x, y);
				maxHeight = std::max(height, maxHeight);

				heights[y * NOISE_TEX_SIZE + x] = height;
			}
		}

		for (int x = 0; x < NOISE_TEX_SIZE; ++x)
		{
			for (int y = 0; y < NOISE_TEX_SIZE; ++y)
			{
				int i = y * NOISE_TEX_SIZE + x;
				int height = heights[i];

				unsigned char byteVal = static_cast<unsigned char>((float)height / maxHeight * 255.f);

				int resultIndex = i * 4;
				result[resultIndex] = byteVal;
				result[resultIndex + 1] = byteVal;
				result[resultIndex + 2] = byteVal;
				result[resultIndex + 3] = alpha;
			}
		}

		unsigned error = lodepng::encode(NOISE_TEX_PATH, result, NOISE_TEX_SIZE, NOISE_TEX_SIZE);
		if (error)
		{
			std::cout << "Error " << error << ": " << lodepng_error_text(error) << std::endl;
		}

		delete[] heights;
	}

	void NoiseTool::draw()
	{
		m_window.update();

		std::vector<g::Mesh*> orthoMeshes = std::vector<g::Mesh*>{ m_tex->getMesh() };

		m_renderer.renderOrtho(orthoMeshes, &m_shader, m_camera);
	}

	bool NoiseTool::shouldClose()
	{
		return glfwWindowShouldClose(m_window.getWindow());
	}
}