#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include "worldGenerator.hpp"
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "../common/threadSafeMap.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	class SplineBasedWorldGenerator : public WorldGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;
		std::unordered_map<int, std::unordered_map<int, int>> m_heightMap;
		std::unordered_map<int, std::unordered_map<int, float>> m_noiseMap;

		int calculateHeight(int x, int z);
		float calculateNoise(int x, int z);

	public:
		SplineBasedWorldGenerator(long seed);
		
		BlockTypeId getBlockType(BlockPos pos);
		void generateChunkData(Chunk& chunk);
		
		int getHeight(int x, int z);
		float getNoise(int x, int z);
		float getContinentalness(int x, int z);
		float getPeaksAndValleys(int x, int z);
		float getErosion(int x, int z);
		float getSquishFactor(int x, int z);
	};
}