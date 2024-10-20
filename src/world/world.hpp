#pragma once

#include <map>
#include <set>
#include <thread>
#include "block.hpp"
#include "chunkManager.hpp"
#include "worldGenerator.hpp"
#include "../player.hpp"
#include "../graphics/shader.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"
#include "../util/threadPool.hpp"
#include "../constants.hpp"

#include <glm/gtx/string_cast.hpp>

namespace voxel_game::world
{
	struct RaycastResult
	{
		Block block;
		g::Direction hitFace;
	};

	class World
	{
	private:
		ChunkManager m_chunkManager;
		WorldGenerator m_worldGenerator;
		utils::ThreadPool m_threadPool;
		g::Shader* m_shader;

		Player m_player;
		BlockPos m_blockBeingBroken;
		int m_breakBlockProgress = 0;

		void removeBlock(const BlockPos& blockPos);
		void putBlock(const Block& block);
		bool canPlaceBlock(const Block& block);

		BlockPos getChunkCoord(BlockPos pos);
		// TODO: rename
		RaycastResult getBlockLookingAt();
		// Return the of the first block that intersects with the ray
		RaycastResult raycast(const glm::vec3 startPos, const glm::vec3 rayDir, int maxDist, int iterationsPerBlock);

		void updateChunkMesh(Chunk* chunk);
		void generateChunk(BlockPos chunkCoord);

		bool allChunksGenerated();

		bool chunkIsFurtherFromPlayer(Chunk* chunk1, Chunk* chunk2, const Player& player);

	public:
		World(WorldGenerator &worldGenerator, g::Shader* shader, Player &player);
		~World();
		void generate();
		void update();
		std::vector<Chunk *> getVisibleChunks();
		BlockTypeId getBlock(const BlockPos& blockPos);
	};

	class ChunkGenerator
	{
	public:
		ChunkGenerator(Chunk &chunk, ChunkManager &chunkManager) : m_chunk(chunk), m_chunkManager(chunkManager) {}

		void run()
		{
			const auto start = std::chrono::system_clock::now();
			log::info("Generating chunk mesh for chunk at " + m_chunk.getChunkCoord());

			m_chunk.updateMesh(m_chunkManager);

			const auto end = std::chrono::system_clock::now();
			int durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			log::info("Finished updating mesh for chunk at " + m_chunk.getChunkCoord() + ". Total vertices: " + std::to_string(m_chunk.getMesh()->getVertexCount()) + ". Duration:" + std::to_string(durationMs) + "ms");
		}

	private:
		Chunk &m_chunk;
		ChunkManager &m_chunkManager;
	};

	BlockPos worldPosToLocalPos(const BlockPos& worldPos);
}