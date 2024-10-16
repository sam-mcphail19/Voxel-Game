#include "chunkManager.hpp"

namespace voxel_game::world
{
	ChunkManager::ChunkManager() {}

	void ChunkManager::putChunk(BlockPos chunkCoord, Chunk *chunk)
	{
		m_chunks.emplace(chunkCoord, chunk);
	}

	Chunk *ChunkManager::getChunk(BlockPos chunkCoord)
	{
		if (m_chunks.find(chunkCoord) == m_chunks.end())
		{
			return NULL;
		}

		return m_chunks.at(chunkCoord);
	}

	std::vector<Chunk *> ChunkManager::getChunks()
	{
		std::vector<Chunk *> chunks;
		for (const auto &chunk : m_chunks)
			chunks.push_back(chunk.second);

		return chunks;
	}

	bool ChunkManager::containsChunk(BlockPos chunkCoord)
	{
		return m_chunks.find(chunkCoord) != m_chunks.end();
	}
}