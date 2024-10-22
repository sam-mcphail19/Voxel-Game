#include "worldGenerator.hpp"

namespace voxel_game::world
{
	static const glm::vec2 continentalnessPoints[] = {
		glm::vec2(0.0f, 80.0f),
		glm::vec2(0.05f, 40.0f),
		glm::vec2(0.4f, 60.0f),
		glm::vec2(0.5f, 75.0f),
		glm::vec2(0.65f, 90.0f),
		glm::vec2(0.8f, 110.0f),
		glm::vec2(1.0f, 120.0f),
	};

	static const glm::vec2 peaksAndValleysPoints[] = {
		glm::vec2(0.0f, -30.0f),
		glm::vec2(0.25f, -10.f),
		glm::vec2(0.5f, 0.0f),
		glm::vec2(0.7f, 50.0f),
		glm::vec2(1.f, 85.f),
	};

	static const glm::vec2 erosionPoints[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.4f, 0.8f),
		glm::vec2(0.5f, 0.7f),
		glm::vec2(0.7f, 0.5f),
		glm::vec2(0.85f, 0.2f),
		glm::vec2(1.f, 0.1f),
	};

	static const int continentalnessPointCount = 7;
	static const int peaksAndValleysPointCount = 5;
	static const int erosionPointCount = 6;
	static const int erosionCels = 8;

	WorldGenerator::WorldGenerator(long seed) : m_noiseGenerator(NoiseGenerator(seed))
	{
		m_heightMapMutex = new std::mutex();
	}

	void WorldGenerator::generateChunkData(Chunk& chunk)
	{
		const auto start = std::chrono::system_clock::now();
		BlockPos origin = chunk.getOrigin();
		for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
		{
			BlockPos pos = to3dIndex(i);
			chunk.putBlock(Block{ pos, getBlockType(origin + pos) });
		}
		const auto end = std::chrono::system_clock::now();
		int durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		log::info("Generating chunk data took " + std::to_string(durationMs) + "ms");
	}

	BlockTypeId WorldGenerator::getBlockType(BlockPos pos)
	{
		if (pos.y < 4)
		{
			return BlockTypeId::BEDROCK;
		}

		int height = getHeight(pos.x, pos.z);

		if (pos.y > height)
		{
			if (pos.y > WATER_HEIGHT)
			{
				return BlockTypeId::AIR;
			}
			return BlockTypeId::WATER;
		}

		if (pos.y > MOUNTAIN_HEIGHT)
		{
			return BlockTypeId::STONE;
		}

		if (pos.y == height)
		{

			if (pos.y > WATER_HEIGHT)
			{
				return BlockTypeId::GRASS;
			}

			if (pos.y == WATER_HEIGHT)
			{
				return BlockTypeId::SAND;
			}

			return BlockTypeId::DIRT;
		}

		if (pos.y > height - 3)
		{
			return BlockTypeId::DIRT;
		}

		return BlockTypeId::STONE;
	}

	int WorldGenerator::getHeight(int x, int z)
	{
		int key = getHeightMapHash(x, z);
		std::unique_lock<std::mutex> lock(*m_heightMapMutex);

		if (m_heightMap.find(key) != m_heightMap.end())
		{
			return m_heightMap[key];
		}

		float noise = getContinentalness(x, z) + getPeaksAndValleys(x, z) * getErosion(x, z);
		//float noise = getContinentalness(x, z);
		//float noise = 100 - getErosion(x, z) * 100;
		//float noise = (getPeaksAndValleys(x, z) + 60) * 100;

		int height = (int)noise;
		m_heightMap[key] = height;
		return height;
	}

	float WorldGenerator::getContinentalness(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.03f, 2.f, 0.5f, 8);
		return utils::evaluate(continentalnessPoints, continentalnessPointCount, noise);
	}

	float WorldGenerator::getPeaksAndValleys(int x, int z)
	{
		float noise = fabs(m_noiseGenerator.noise2(x, z, 0.035f, 2.f, 0.5f, 6) * 2 - 1);
		return utils::evaluate(peaksAndValleysPoints, peaksAndValleysPointCount, noise);
	}

	float WorldGenerator::getErosion(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.01f, 2.f, 0.5f, 6);
		return utils::evaluate(erosionPoints, erosionPointCount, noise);
	}

	int WorldGenerator::getHeightMapHash(int x, int z)
	{
		return CHUNK_BLOCK_COUNT * x + z;
	}
}