#pragma once

#include <mutex>
#include <glm/vec3.hpp>
#include "block.hpp"
#include "chunkManager.hpp"
#include "cube.hpp"
#include "../constants.hpp"
#include "../graphics/mesh.hpp"

namespace voxel_game::world
{
	class World;

	class Chunk
	{
	private:
		BlockPos m_origin;
		World *m_world;
		BlockTypeId *m_blocks;
		graphics::Mesh *m_mesh = NULL;

		bool isFaceVisible(BlockPos pos, graphics::Direction direction, ChunkManager &chunkManager);
		Chunk *getNeighbourChunk(graphics::Direction direction, ChunkManager &chunkManager);

	public:
		Chunk(BlockPos chunkCoord, World *world);
		void updateMesh(ChunkManager &chunkManager);
		void putBlock(Block block);
		BlockTypeId getBlock(int x, int y, int z);
		BlockTypeId getBlock(BlockPos blockPos);
		BlockPos getOrigin();
		BlockPos getChunkCoord();
		graphics::Mesh *getMesh();
	};

	int to1dIndex(int x, int y, int z);
	int to1dIndex(BlockPos pos);
	BlockPos to3dIndex(int i);
	glm::vec3 toVec3(BlockPos blockPos);
}
