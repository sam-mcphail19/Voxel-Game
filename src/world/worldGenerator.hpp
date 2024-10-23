#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	class WorldGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

		BlockTypeId getBlockType(BlockPos pos, std::vector<std::vector<int>>& heightMap);
		BlockTypeId getBlockType(BlockPos pos, std::function<int()> getHeightFunc);
		int getHeight(int x, int z, std::vector<std::vector<int>>& heightMap);

		float getContinentalness(int x, int z);
		float getPeaksAndValleys(int x, int z);
		float getErosion(int x, int z);

	public:
		WorldGenerator(long seed);
		BlockTypeId getBlockType(BlockPos pos);
		int getHeight(int x, int z);
		void generateChunkData(Chunk &chunk);
	};
}