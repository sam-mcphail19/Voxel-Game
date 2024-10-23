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
		std::set<BlockPos> m_generatingChunks;
		std::mutex m_generatingChunksMutex;
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
		void updateChunkMeshAsync(Chunk* chunk);
		void generateChunk(BlockPos chunkCoord);
		void generateChunkAsync(BlockPos chunkCoord);

		std::vector<BlockPos> getChunksToGenerate();
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

	BlockPos worldPosToLocalPos(const BlockPos& worldPos);
}