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

	WorldGenerator::WorldGenerator(long seed) : m_noiseGenerator(NoiseGenerator(seed)) {}

	void WorldGenerator::generateChunkData(Chunk& chunk)
	{
		const auto start = std::chrono::system_clock::now();

		std::vector<std::vector<int>> heightMap(CHUNK_SIZE, std::vector<int>(CHUNK_SIZE));
		for (int i = 0; i < CHUNK_SIZE; i++) {
			for (int j = 0; j < CHUNK_SIZE; j++) {
				heightMap[i][j] = -1;
			}
		}

		BlockPos origin = chunk.getOrigin();

		for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
		{
			BlockPos pos = to3dIndex(i);
			chunk.putBlock(Block{ pos, getBlockType(origin + pos, heightMap) });
		}

		const auto end = std::chrono::system_clock::now();
		int durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		log::info("Generating chunk data took " + std::to_string(durationMs) + "ms");
	}

	BlockTypeId WorldGenerator::getBlockType(BlockPos pos, std::vector<std::vector<int>>& heightMap)
	{
		return getBlockType(pos, [this, &pos, &heightMap]() {
			return getHeight(pos.x, pos.z, heightMap);
			}
		);
	}

	BlockTypeId WorldGenerator::getBlockType(BlockPos pos)
	{
		return getBlockType(pos, [this, &pos]() {
			return getHeight(pos.x, pos.z);
			}
		);
	}

	BlockTypeId WorldGenerator::getBlockType(BlockPos pos, std::function<int()> getHeightFunc)
	{
		if (pos.y < 4)
		{
			return BlockTypeId::BEDROCK;
		}

		int height = getHeightFunc();

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

	int WorldGenerator::getHeight(int x, int z, std::vector<std::vector<int>>& heightMap)
	{
		// TODO: this is prob way less efficient than just passing the chunk's origin
		int localX = x;
		int localZ = z;

		while (localX < 0)
			localX += CHUNK_SIZE;
		while (localZ < 0)
			localZ += CHUNK_SIZE;

		localX %= CHUNK_SIZE;
		localZ %= CHUNK_SIZE;

		int cachedVal = heightMap[localX][localZ];
		if (cachedVal != -1)
		{
			return cachedVal;
		}

		heightMap[localX][localZ] = getHeight(x, z);
		return heightMap[localX][localZ];
	}

	int WorldGenerator::getHeight(int x, int z)
	{
		float noise = getContinentalness(x, z) + getPeaksAndValleys(x, z) * getErosion(x, z);
		//float noise = getContinentalness(x, z);
		//float noise = 100 - getErosion(x, z) * 100;
		//float noise = (getPeaksAndValleys(x, z) + 60) * 100;

		return (int)noise;
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
}