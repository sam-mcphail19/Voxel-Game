#pragma once

#include <map>
#include <mutex>
#include <vector>
#include "block.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	class Chunk;

	class ChunkManager
	{
	private:
		std::map<BlockPos, Chunk *> m_chunks;
		std::mutex m_lock;

	public:
		ChunkManager();
		void putChunk(BlockPos chunkCoord, Chunk* chunk);
		Chunk *getChunk(BlockPos chunkCoord);
		std::vector<Chunk*> getChunks();
		bool containsChunk(BlockPos chunkCoord);
	};
}