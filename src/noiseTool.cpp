#include "noiseTool.hpp"

namespace voxel_game
{
	const std::unordered_map<world::BlockTypeId, BlockPixel> blockColors = {
			{world::BlockTypeId::WATER, BlockPixel{30_uchar, 30_uchar, 255_uchar}},
			{world::BlockTypeId::STONE, BlockPixel{200_uchar, 200_uchar, 200_uchar}},
			{world::BlockTypeId::DIRT, BlockPixel{170_uchar, 40_uchar, 40_uchar}},
			{world::BlockTypeId::GRASS, BlockPixel{30_uchar, 220_uchar, 100_uchar}},
			{world::BlockTypeId::SAND, BlockPixel{200_uchar, 180_uchar, 130_uchar}},
			{world::BlockTypeId::GRAVEL, BlockPixel{150_uchar, 150_uchar, 150_uchar}},
	};

	NoiseTool::NoiseTool()
	{
		m_window.setBackground(0.05, 0.05, 0.05);
		m_transform = new physics::Transform(
			glm::vec3(-0.5, -0.5, 0),
			glm::mat4(1),
			glm::vec3(1, 1, 1)
		);
	}

	void NoiseTool::generate(long seed)
	{
		m_seed = seed;
		sampleSeed(seed);
		renderCurrentView();
	}

	void NoiseTool::cycleView()
	{
		static constexpr std::array<const char*, static_cast<int>(NoiseView::Count)> viewNames = {
			"Surface",
			"Continentalness",
			"Erosion",
			"Temperature",
			"Humidity",
			"Ridges",
			"Coasts",
			"Slope",
			"Plateau cliffs",
			"3D formations",
			"Rivers",
			"Terrain types",
			"Biomes",
			"Height"
		};
		int nextView = (static_cast<int>(m_view) + 1) % static_cast<int>(NoiseView::Count);
		m_view = static_cast<NoiseView>(nextView);
		log::info(std::string("Noise view: ") + viewNames[nextView]);
		renderCurrentView();
	}

	void NoiseTool::sampleSeed(long seed)
	{
		const auto start = std::chrono::steady_clock::now();
		m_samples.resize(NOISE_TEX_SIZE * NOISE_TEX_SIZE);
		std::atomic<int> nextRow = 0;
		unsigned int threadCount = std::max(1u, std::thread::hardware_concurrency());
		std::vector<std::thread> workers;
		workers.reserve(threadCount);
		auto worldGenerator = std::make_shared<world::BiomeBasedWorldGenerator>(seed);

		for (unsigned int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
		{
			workers.emplace_back([&]()
				{
					while (true)
					{
						int y = nextRow.fetch_add(1);
						if (y >= NOISE_TEX_SIZE) break;
						for (int x = 0; x < NOISE_TEX_SIZE; ++x)
						{
							m_samples[y * NOISE_TEX_SIZE + x] = worldGenerator->getTerrainDebugSample(x, y);
						}
					}
				});
		}

		for (auto& worker : workers) worker.join();

		for (int y = 0; y < NOISE_TEX_SIZE; ++y)
		{
			for (int x = 0; x < NOISE_TEX_SIZE; ++x)
			{
				auto& sample = m_samples[y * NOISE_TEX_SIZE + x];
				int west = m_samples[y * NOISE_TEX_SIZE + std::max(0, x - 1)].surfaceHeight;
				int east = m_samples[y * NOISE_TEX_SIZE + std::min(NOISE_TEX_SIZE - 1, x + 1)].surfaceHeight;
				int north = m_samples[std::max(0, y - 1) * NOISE_TEX_SIZE + x].surfaceHeight;
				int south = m_samples[std::min(NOISE_TEX_SIZE - 1, y + 1) * NOISE_TEX_SIZE + x].surfaceHeight;
				sample.slope = 0.5f * std::max(std::abs(east - west), std::abs(south - north));
				sample.surfaceBlock = world::BiomeBasedWorldGenerator::selectSurfaceBlock(
					sample.surfaceBlock, sample.coast, sample.slope, sample.surfaceHeight);
				if (sample.rivers > 0.45f)
				{
					sample.surfaceBlock = world::BlockTypeId::WATER;
				}
			}
		}
		auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		log::info("Sampled noise seed in " + std::to_string(durationMs) + "ms using " + std::to_string(threadCount) + " threads");
	}

	void NoiseTool::renderCurrentView()
	{
		const auto start = std::chrono::steady_clock::now();

		std::vector<unsigned char> result(NOISE_TEX_SIZE * NOISE_TEX_SIZE * 4, 255);
		const std::array<BlockPixel, 5> terrainColors = {
			BlockPixel{80_uchar, 180_uchar, 80_uchar},
			BlockPixel{130_uchar, 190_uchar, 70_uchar},
			BlockPixel{150_uchar, 120_uchar, 70_uchar},
			BlockPixel{220_uchar, 150_uchar, 70_uchar},
			BlockPixel{210_uchar, 210_uchar, 220_uchar}
		};

		for (int y = 0; y < NOISE_TEX_SIZE; ++y)
		{
			for (int x = 0; x < NOISE_TEX_SIZE; ++x)
			{
				const auto& sample = m_samples[y * NOISE_TEX_SIZE + x];
				BlockPixel blockColor{};
				if (m_view == NoiseView::Surface)
				{
					blockColor = blockColors.at(sample.surfaceBlock);
				}
				else
				{
					if (m_view == NoiseView::TerrainTypes)
					{
						float r = 0.0f, g = 0.0f, b = 0.0f;
						for (size_t terrainIndex = 0; terrainIndex < sample.terrainWeights.size(); ++terrainIndex)
						{
							r += terrainColors[terrainIndex].r * sample.terrainWeights[terrainIndex];
							g += terrainColors[terrainIndex].g * sample.terrainWeights[terrainIndex];
							b += terrainColors[terrainIndex].b * sample.terrainWeights[terrainIndex];
						}
						blockColor = { static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b) };
					}
					else if (m_view == NoiseView::Biomes)
					{
						switch (sample.biome)
						{
						case world::BiomeType::Ocean: blockColor = { 35_uchar, 80_uchar, 210_uchar }; break;
						case world::BiomeType::Desert: blockColor = { 225_uchar, 195_uchar, 105_uchar }; break;
						case world::BiomeType::Mountains: blockColor = { 150_uchar, 150_uchar, 155_uchar }; break;
						case world::BiomeType::Plains: blockColor = { 65_uchar, 190_uchar, 75_uchar }; break;
						}
					}
					else
					{
						float value = 0.0f;
						switch (m_view)
						{
						case NoiseView::Continentalness: value = sample.continentalness; break;
						case NoiseView::Erosion: value = sample.erosion; break;
						case NoiseView::Temperature: value = sample.temperature; break;
						case NoiseView::Humidity: value = sample.humidity; break;
						case NoiseView::Ridges: value = sample.ridge; break;
						case NoiseView::Coasts: value = sample.coast; break;
						case NoiseView::Slope: value = sample.slope / 6.0f; break;
						case NoiseView::PlateauCliffs: value = sample.plateauCliffs; break;
						case NoiseView::Formations: value = sample.formations; break;
						case NoiseView::Rivers: value = sample.rivers; break;
						case NoiseView::Height:
							value = (sample.height - MIN_WORLD_GEN_HEIGHT) / static_cast<float>(MAX_WORLD_GEN_HEIGHT - MIN_WORLD_GEN_HEIGHT);
							break;
						default: break;
						}
						unsigned char grey = static_cast<unsigned char>(glm::clamp(value, 0.0f, 1.0f) * 255.0f);
						blockColor = { grey, grey, grey };
					}
				}

				int resultIndex = (y * NOISE_TEX_SIZE + x) * 4;
				result[resultIndex] = blockColor.r;
				result[resultIndex + 1] = blockColor.g;
				result[resultIndex + 2] = blockColor.b;
				result[resultIndex + 3] = 255;
			}
		}

		if (m_texture == nullptr)
		{
			m_texture = g::createTexture(NOISE_TEX_SIZE, NOISE_TEX_SIZE, result.data());
			m_tex = g::Quad::createQuad(m_transform, m_texture);
		}
		else
		{
			m_texture->upload(NOISE_TEX_SIZE, NOISE_TEX_SIZE, result.data());
		}

		auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		log::info("Rendered noise view in " + std::to_string(durationMs) + "ms");
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
