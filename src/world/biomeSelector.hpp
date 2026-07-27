#pragma once

#include <vector>
#include "biome.hpp"
#include "terrainGenerator.hpp"

namespace voxel_game::world
{
	struct BiomeWeight
	{
		const Biome* biome;
		float weight;

		bool operator<(const BiomeWeight& other) const noexcept
		{
			return weight < other.weight;
		}
	};

	class BiomeSelector
	{
	private:
		NoiseGenerator m_noiseGenerator;

	public:
		explicit BiomeSelector(long seed);

		std::vector<BiomeWeight> buildWeights(const TerrainSample& terrain) const;
		std::vector<BiomeWeight> normalize(std::vector<BiomeWeight> weights) const;
		float getWeight(const TerrainSample& terrain, BiomeType biome) const;
		const Biome* select(std::vector<BiomeWeight> weights, int x, int z);
	};
}
