#include "splineBasedWorldGenerator.hpp"

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

	static const glm::vec2 squishFactorPoints[] = {
		glm::vec2(0.00f, 0.70f),
		glm::vec2(0.50f, 0.90f),
		glm::vec2(1.00f, 1.00f),
	};

	static const int continentalnessPointCount = 11;
	static const int peaksAndValleysPointCount = 12;
	static const int erosionPointCount = 7;
	static const int squishFactorPointCount = 3;

	static const float continentalnessPower = 0.95f;
	static const float peaksAndValleysPower = 1.2f;

	SplineBasedWorldGenerator::SplineBasedWorldGenerator(long seed) : m_noiseGenerator(NoiseGenerator(seed)) {}

	void SplineBasedWorldGenerator::generateChunkData(Chunk& chunk)
	{
		const auto start = std::chrono::system_clock::now();

		BlockPos origin = chunk.getOrigin();

		for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
		{
			BlockPos pos = to3dIndex(i);
			chunk.putBlock(Block{ pos, getBlockType(origin + pos) });
		}

		const auto end = std::chrono::system_clock::now();
		auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		log::info("Generating chunk data took " + std::to_string(durationMs) + "ms");
	}

	BlockTypeId SplineBasedWorldGenerator::getBlockType(BlockPos pos)
	{
		if (pos.y < 4)
		{
			return BlockTypeId::BEDROCK;
		}

		int baseLevel = getHeight(pos.x, pos.z);

		float densityMod = ((baseLevel - pos.y) / (float)baseLevel) * getSquishFactor(pos.x, pos.z);
		float density = m_noiseGenerator.noise3(pos.x, pos.y, pos.z, 0.09f, 2.0f, 0.5f, 5);

		if (density + densityMod > 0.5f)
		{
			return BlockTypeId::STONE;
		}

		return pos.y > WATER_HEIGHT ? BlockTypeId::AIR : BlockTypeId::WATER;
	}

	int SplineBasedWorldGenerator::getHeight(int x, int z)
	{
		if (m_heightMap.contains(x) && m_heightMap.at(x).contains(z))
		{
			return m_heightMap.at(x).at(z);
		}

		int height = calculateHeight(x, z);
		if (!m_heightMap.contains(x))
		{
			m_heightMap[x] = std::unordered_map<int, int>();
		}

		m_heightMap[x][z] = height;

		return height;
	}

	int SplineBasedWorldGenerator::calculateHeight(int x, int z)
	{
		return (int)utils::lerp(MIN_WORLD_GEN_HEIGHT, MAX_WORLD_GEN_HEIGHT, getNoise(x, z));
	}

	float SplineBasedWorldGenerator::calculateNoise(int x, int z)
	{
		float c = getContinentalness(x, z);
		float pv = getPeaksAndValleys(x, z);
		float e = getErosion(x, z);

		// At higher erosion, bias towards the c value. At lower erosion, bias towards pv
		return utils::lerp(pv, c, e);
	}

	float SplineBasedWorldGenerator::getNoise(int x, int z)
	{
		if (m_noiseMap.contains(x) && m_noiseMap.at(x).contains(z))
		{
			return m_noiseMap.at(x).at(z);
		}

		float noise = calculateNoise(x, z);

		if (!m_noiseMap.contains(x))
		{
			m_noiseMap[x] = std::unordered_map<int, float>();
		}

		m_noiseMap[x][z] = noise;

		return noise;
	}

	// TODO: use different evaluate functions based on biome
	float SplineBasedWorldGenerator::getContinentalness(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.055f, 2.0f, 0.7f, 7);
		noise = powf(noise, continentalnessPower);

		return utils::evaluate(continentalnessPoints, continentalnessPointCount, noise);
	}

	float SplineBasedWorldGenerator::getPeaksAndValleys(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.06f, 2.0f, 0.4f, 6);

		noise = powf(noise, peaksAndValleysPower);
		noise = fabs(noise * 2.f - 1);

		return utils::evaluate(peaksAndValleysPoints, peaksAndValleysPointCount, noise);
	}

	float SplineBasedWorldGenerator::getErosion(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.03f, 2.0f, 0.35f, 6);

		return utils::evaluate(erosionPoints, erosionPointCount, noise);
	}

	float SplineBasedWorldGenerator::getSquishFactor(int x, int z)
	{
		float noise = m_noiseGenerator.noise2(x, z, 0.2f, 2.0f, 0.55f, 4);

		return utils::evaluate(squishFactorPoints, squishFactorPointCount, noise);
	}
}