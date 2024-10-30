#include "worldGenerator.hpp"

namespace voxel_game::world
{
	static const glm::vec2 continentalnessPoints[] = {
		glm::vec2(0.00f, 0.04f),
		glm::vec2(0.15f, 0.07f),
		glm::vec2(0.44f, 0.09f),
		glm::vec2(0.50f, 0.21f),
		glm::vec2(0.59f, 0.26f),
		glm::vec2(0.71f, 0.44f),
		glm::vec2(0.74f, 0.54f),
		glm::vec2(0.77f, 0.65f),
		glm::vec2(0.81f, 0.67f),
		glm::vec2(0.88f, 0.68f),
		glm::vec2(1.00f, 0.71f),
	};

	static const glm::vec2 peaksAndValleysPoints[] = {
		glm::vec2(0.00f, 0.09f),
		glm::vec2(0.06f, 0.10f),
		glm::vec2(0.07f, 0.12f),
		glm::vec2(0.10f, 0.17f),
		glm::vec2(0.21f, 0.19f),
		glm::vec2(0.45f, 0.26f),
		glm::vec2(0.52f, 0.31f),
		glm::vec2(0.62f, 0.38f),
		glm::vec2(0.70f, 0.54f),
		glm::vec2(0.77f, 0.67f),
		glm::vec2(0.89f, 0.78f),
		glm::vec2(1.00f, 0.85f),
	};

	static const glm::vec2 erosionPoints[] = {
		glm::vec2(0.00f, 0.10f),
		glm::vec2(0.10f, 0.10f),
		glm::vec2(0.20f, 0.13f),
		glm::vec2(0.35f, 0.24f),
		glm::vec2(0.60f, 0.44f),
		glm::vec2(0.80f, 0.60f),
		glm::vec2(1.00f, 0.90f),
	};

	static const int continentalnessPointCount = 11;
	static const int peaksAndValleysPointCount = 12;
	static const int erosionPointCount = 7;

	static const float continentalnessPower = 0.95f;
	static const float peaksAndValleysPower = 1.2f;

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
		// using getHeight() height, apply squishing as in https://www.reddit.com/r/VoxelGameDev/comments/zedp39/how_does_minecraft_use_2d_and_3d_noise_to/

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
		return utils::lerp(MIN_WORLD_GEN_HEIGHT, MAX_WORLD_GEN_HEIGHT, getNoise(x, z));
	}

	float WorldGenerator::getNoise(int x, int z)
	{
		float c = getContinentalness(x, z);
		float pv = getPeaksAndValleys(x, z);
		float e = getErosion(x, z);

		// At higher erosion, bias towards the c value. At lower erosion, bias towards pv
		return utils::lerp(pv, c, e);
	}

	// TODO: use different evaluate functions based on biome
	float WorldGenerator::getContinentalness(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.055f, 2.f, 0.7f, 7);
		noise = powf(noise, continentalnessPower);

		return utils::evaluate(continentalnessPoints, continentalnessPointCount, noise);
	}

	float WorldGenerator::getPeaksAndValleys(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.06f, 2.f, 0.4f, 6);

		noise = powf(noise, peaksAndValleysPower);
		noise = fabs(noise * 2.f - 1);

		return utils::evaluate(peaksAndValleysPoints, peaksAndValleysPointCount, noise);
	}

	float WorldGenerator::getErosion(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.03f, 2.0f, 0.35f, 6);

		return utils::evaluate(erosionPoints, erosionPointCount, noise);
	}
}