#include "chunkManager.hpp"

namespace voxel_game::world
{
	ChunkManager::ChunkManager() {}

	void ChunkManager::putChunk(BlockPos chunkCoord, Chunk *chunk)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_chunks.emplace(chunkCoord, chunk);
	}

	Chunk *ChunkManager::getChunk(BlockPos chunkCoord)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		if (m_chunks.find(chunkCoord) == m_chunks.end())
		{
			return NULL;
		}

		return m_chunks.at(chunkCoord);
	}

	std::vector<Chunk *> ChunkManager::getChunks()
	{
		std::vector<Chunk *> chunks;
		std::lock_guard<std::mutex> lock(m_lock);
		for (const auto &chunk : m_chunks)
			chunks.push_back(chunk.second);

		return chunks;
	}

	bool ChunkManager::containsChunk(BlockPos chunkCoord)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		return m_chunks.find(chunkCoord) != m_chunks.end();
	}
}