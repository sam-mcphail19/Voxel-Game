#include "biomeBasedWorldGenerator.hpp"

namespace voxel_game::world
{
	static float hash01(int x, int z) {
		uint32_t h = uint32_t(x)*0x8DA6B343u
				^ uint32_t(z)*0xD8163841u;
		h = (h ^ (h >> 16)) * 0x45D9F3Bu;
		h = (h ^ (h >> 16)) * 0x45D9F3Bu;
		h = h ^ (h >> 16);
		return float(h & 0xFFFFFF) / float(0x1000000);
	}

	BiomeBasedWorldGenerator::BiomeBasedWorldGenerator(long seed)
		: m_noiseGenerator(seed) {}

	//TODO: refactor so this goes with the other biome specific definitions
	static BiomeWeight scoreOcean(float cont, float temp, float hum, float erosion)
	{
		float w = utils::sigmoid(1.0f, 0.4f, -15.0f, cont);

		std::vector<float> weights = {w};

		return {BIOMES.at(BiomeType::Ocean), utils::average(weights)};
	}

	static BiomeWeight scoreDesert(float cont, float temp, float hum, float erosion)
	{
		float wt = glm::smoothstep(0.6f, 1.0f, temp);
		float wh = 1 - glm::smoothstep(0.0f, 0.3f, hum);

		std::vector<float> weights = {wt, wh};

		return {BIOMES.at(BiomeType::Desert), utils::average(weights)};
	}

	static BiomeWeight scoreMountain(float cont, float temp, float hum, float erosion)
	{
		float wc = utils::sigmoid(1.0f, 0.65f, 20.0f, cont);
		float we = std::pow(100.0f, -erosion);

		std::vector<float> weights = {wc, we};

		return {BIOMES.at(BiomeType::Mountains), utils::average(weights)};
	}

	static BiomeWeight scorePlains(float cont, float temp, float hum, float erosion)
	{
		float wc = 1 - 10 * std::pow(cont - 0.7f, 2);
		float we = std::pow(100.0f, erosion - 0.8f);

		std::vector<float> weights = {wc, we};

		return {BIOMES.at(BiomeType::Plains), utils::average(weights)};
	}

	float BiomeBasedWorldGenerator::calcContinentalness(int x, int z)
	{
		return m_noiseGenerator.noise2(x + 5555, z + 5555, 0.005f, 2.0f, 0.5f, 3);
	}

	float BiomeBasedWorldGenerator::calcErosion(int x, int z)
	{
		return m_noiseGenerator.noise2(x + 1111, z + 1111, 0.0005f, 2.0f, 0.5f, 3);
	}

	float BiomeBasedWorldGenerator::calcTemperature(int x, int z)
	{
		return m_noiseGenerator.noise2(x, z, 0.001f, 2.0f, 0.5f, 4);
	}

	float BiomeBasedWorldGenerator::calcHumidity(int x, int z)
	{
		return m_noiseGenerator.noise2(x + 9999, z + 9999, 0.02f, 2.0f, 0.5f, 4);
	}

	std::vector<BiomeWeight> BiomeBasedWorldGenerator::buildWeights(int x, int z)
	{
		float cont = calcContinentalness(x, z);
		float erosion = calcErosion(x, z);
		float temp = calcTemperature(x, z);
		float hum = calcHumidity(x, z);

		BiomeWeight ocean = scoreOcean(cont, temp, hum, erosion);
		BiomeWeight desert = scoreDesert(cont, temp, hum, erosion);
		BiomeWeight mountains = scoreMountain(cont, temp, hum, erosion);
		BiomeWeight plains = scorePlains(cont, temp, hum, erosion);

		return std::vector<BiomeWeight>{
			ocean, 
			desert,
			mountains,
			plains
		};
	}

	std::vector<BiomeWeight> BiomeBasedWorldGenerator::normalizeWeights(std::vector<BiomeWeight> weights)
	{
		float sum = 0.f;
		for (auto &biomeWeight : weights)
		{
			sum += biomeWeight.weight;
		}
		if (sum > 0.f)
		{
			for (auto &biomeWeight : weights)
			{
				biomeWeight.weight /= sum;
			}
		}
		return weights;
	}

	int BiomeBasedWorldGenerator::calculateHeight(int x, int z)
	{
		auto biomeWeights = buildWeights(x, z);
		biomeWeights = normalizeWeights(biomeWeights);
		float totalHeight = 0.f;
		for (auto &biomeWeight : biomeWeights)
		{
			totalHeight += biomeWeight.weight * biomeWeight.biome->getHeight(x, z, m_noiseGenerator);
		}
		return static_cast<int>(totalHeight);
	}

	int BiomeBasedWorldGenerator::getHeight(int x, int z)
	{
		return m_heightMap.withLock([&](auto &map) -> int {
			auto &row = map[x];
			if (auto it = row.find(z); it != row.end())
			{
				return it->second;
			}
        	return row[z] = calculateHeight(x, z);
		});
	}

	BlockTypeId BiomeBasedWorldGenerator::getBlockType(BlockPos pos)
	{
		int x = pos.x, z = pos.z, y = pos.y;
		int height = getHeight(x, z);

		if (y > height)
		{
			return y <= WATER_HEIGHT ? BlockTypeId::WATER : BlockTypeId::AIR;
		}

		auto weights = buildWeights(x, z);

		return std::max_element(weights.begin(), weights.end())->biome->blockFunc(x, y, z, height);
	}

	void BiomeBasedWorldGenerator::generateChunkData(Chunk &chunk)
	{
		auto start = std::chrono::system_clock::now();
		auto origin = chunk.getOrigin();

		for (int i = 0; i < CHUNK_BLOCK_COUNT; ++i)
		{
			BlockPos local = to3dIndex(i);
			BlockPos world = origin + local;
			chunk.putBlock(Block{local, getBlockType(world)});
		}

		auto end = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		log::info("Generating chunk data took " + std::to_string(ms) + "ms");
	}

}