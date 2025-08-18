#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include "biome.hpp"
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "../common/threadSafeMap.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	class WorldGenerator
	{
	public:
		virtual int getHeight(int x, int z);
		virtual BlockTypeId getBlockType(BlockPos pos);
		virtual void generateChunkData(Chunk& chunk);
	};
}