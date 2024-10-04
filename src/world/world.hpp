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
#include "../constants.hpp"

#include <glm/gtx/string_cast.hpp>

namespace voxel_game::world
{
	class World
	{
	private:
		ChunkManager m_chunkManager;
		WorldGenerator m_worldGenerator;
		g::Shader* m_shader;

		Player m_player;

		BlockTypeId getBlock(BlockPos blockPos);
		BlockPos getChunkCoord(BlockPos pos);
		// TODO: rename
		Block getBlockLookingAt();
		// Return the of the first block that intersects with the ray
		// TODO: rename
		Block raycast(const glm::vec3 startPos, const glm::vec3 rayDir, int maxDist, int iterationsPerBlock);

		void generateChunk(BlockPos chunkCoord);

	public:
		World(WorldGenerator &worldGenerator, g::Shader* shader, Player &player);
		void generate();
		void update();
		std::vector<Chunk *> getVisibleChunks();
	};

	class ChunkGenerator
	{
	public:
		ChunkGenerator(Chunk &chunk, ChunkManager &chunkManager) : m_chunk(chunk), m_chunkManager(chunkManager) {}

		void run()
		{
			log::info("Generating chunk mesh for chunk at " + m_chunk.getChunkCoord());
			m_chunk.updateMesh(m_chunkManager);
			log::info("Finished updating mesh for chunk at " + m_chunk.getChunkCoord() + ". Total vertices: " + std::to_string(m_chunk.getMesh()->getVertexCount()));
		}

	private:
		Chunk &m_chunk;
		ChunkManager &m_chunkManager;
	};

	BlockPos worldPosToLocalPos(const BlockPos worldPos);
}