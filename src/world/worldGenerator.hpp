#pragma once

#include <unordered_map>
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	class WorldGenerator
	{
	private:
		long m_seed;
		NoiseGenerator m_noiseGenerator;
		std::unordered_map<int, int> m_heightMap;

		BlockTypeId getBlockType(BlockPos pos);
		int getHeightMapHash(int x, int z);

	public:
		WorldGenerator(long seed);
		int getHeight(int x, int z);
		void generateChunkData(Chunk &chunk);
	};
}