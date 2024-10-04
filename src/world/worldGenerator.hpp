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
		NoiseGenerator m_noiseGenerator;
		std::unordered_map<int, int> m_heightMap;

		BlockTypeId getBlockType(BlockPos pos);
		int getHeight(int x, int z);
		int getHeightMapHash(int x, int z);

	public:
		WorldGenerator(NoiseGenerator &noiseGenerator);
		void generateChunkData(Chunk &chunk);
	};
}