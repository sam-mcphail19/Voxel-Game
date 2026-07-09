#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <unordered_map>
#include "worldGenerator.hpp"
#include "biome.hpp"
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "../common/threadSafeMap.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	struct BiomeWeight
	{
		const Biome *biome;
		float weight;

		bool operator<(BiomeWeight const &other) const noexcept
		{
			return weight < other.weight;
		}
	};

	class BiomeBasedWorldGenerator : public WorldGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;
		ThreadSafeMap<int, std::unordered_map<int, int>> m_heightMap;

		float calculateNoise(int x, int z);
		int calculateHeight(int x, int z);
		int calculateHeight(std::vector<BiomeWeight>& normalizedWeights, int x, int z);
		const Biome* getDominantBiome(std::vector<BiomeWeight>& weights);

	public:
		BiomeBasedWorldGenerator(long seed);

		float calcContinentalness(int x, int z);
		float calcErosion(int x, int z);
		float calcTemperature(int x, int z);
		float calcHumidity(int x, int z);

		std::vector<BiomeWeight> buildWeights(int x, int z);
		std::vector<BiomeWeight> normalizeWeights(std::vector<BiomeWeight> weights);
		int getHeight(int x, int z);
		BlockTypeId getBlockType(BlockPos pos);
		void generateChunkData(Chunk &chunk);
	};
}
