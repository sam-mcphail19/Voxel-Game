#pragma once

#include <unordered_map>
#include <utility>
#include <glm/vec3.hpp>
#include "block.hpp"
#include "chunkManager.hpp"
#include "cube.hpp"
#include "../constants.hpp"
#include "../graphics/mesh.hpp"

namespace voxel_game::world
{
	class World;

	struct Face {
		BlockPos pos;
		g::Direction dir;
	};

	class Chunk
	{
	private:
		BlockPos m_origin;
		World *m_world;
		BlockTypeId *m_blocks;
		graphics::Mesh *m_mesh = NULL;

		bool isFaceVisible(BlockPos pos, graphics::Direction direction, ChunkManager &chunkManager, std::unordered_map<Face, bool> checkedFaces);
		Chunk *getNeighbourChunk(graphics::Direction direction, ChunkManager &chunkManager);

	public:
		Chunk(BlockPos chunkCoord, World *world);
		void updateMesh(ChunkManager &chunkManager);
		void putBlock(Block block);
		BlockTypeId getBlock(int x, int y, int z);
		BlockTypeId getBlock(BlockPos blockPos);
		BlockPos getOrigin() const;
		BlockPos getChunkCoord();
		graphics::Mesh *getMesh();
	};

	int to1dIndex(int x, int y, int z);
	int to1dIndex(BlockPos pos);
	BlockPos to3dIndex(int i);
	glm::vec3 toVec3(BlockPos blockPos);
}

namespace std {
	template<>
	struct hash<voxel_game::world::Face> {
		size_t operator()(const voxel_game::world::Face& face) const {
			unsigned int hash = (int) face.dir;
			hash *= 37;
			hash += face.pos.x;
			hash *= 37;
			hash += face.pos.y;
			hash *= 37;
			hash += face.pos.z;
			return hash;
		}
	};
}

namespace std {
	template<>
	struct equal_to<voxel_game::world::Face> {
		bool operator()(const voxel_game::world::Face& face1, const voxel_game::world::Face& face2) const {
			return face1.dir == face2.dir && face1.pos == face2.pos;
		}
	};
}