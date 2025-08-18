#pragma once

#include <mutex>
#include <unordered_map>
#include <utility>
#include <memory>
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
		World* m_world;
		BlockTypeId* m_blocks;
		std::shared_ptr<g::Mesh> m_mesh = nullptr;
		std::shared_ptr<g::Mesh> m_transparentMesh = nullptr;
		std::mutex m_mutex;

		bool isBlockInBounds(const BlockPos& blockPos) const;
		bool isFaceVisible(const BlockTypeId& blockTypeId, const Face& face, ChunkManager& chunkManager);
		Chunk* getNeighbourChunk(graphics::Direction direction, ChunkManager& chunkManager);

		void addFaceToMesh(g::Quad* face, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices);

	public:
		Chunk(BlockPos chunkCoord, World* world);
		void updateMesh(ChunkManager& chunkManager);
		void putBlock(Block block);
		BlockTypeId getBlock(int x, int y, int z);
		BlockTypeId getBlock(BlockPos blockPos);
		BlockPos getOrigin() const;
		BlockPos getChunkCoord();
		std::unique_lock<std::mutex> acquireLock();
		std::shared_ptr<g::Mesh> getMesh();
		std::shared_ptr<g::Mesh> getTransparentMesh();
	};

	int to1dIndex(int x, int y, int z);
	int to1dIndex(BlockPos pos);
	BlockPos to3dIndex(int i);
	glm::vec3 toVec3(BlockPos blockPos);
}
