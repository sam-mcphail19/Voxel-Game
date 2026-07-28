#include "chunkManager.hpp"
#include "chunk.hpp"
#include <cstdlib>

namespace voxel_game::world
{
	ChunkManager::ChunkManager() {}

	ChunkManager::~ChunkManager()
	{
		for (const auto& [coord, chunk] : m_chunks)
		{
			delete chunk;
		}
	}

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
			return nullptr;
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

	std::vector<Chunk*> ChunkManager::removeChunksOutside(BlockPos center, int horizontalDistance)
	{
		std::vector<Chunk*> removed;
		std::lock_guard<std::mutex> lock(m_lock);
		for (auto it = m_chunks.begin(); it != m_chunks.end();)
		{
			int dx = std::abs(it->first.x - center.x);
			int dz = std::abs(it->first.z - center.z);
			if (dx > horizontalDistance || dz > horizontalDistance)
			{
				removed.push_back(it->second);
				it = m_chunks.erase(it);
			}
			else
			{
				++it;
			}
		}
		return removed;
	}
}
