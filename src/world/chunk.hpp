#pragma once

#include <algorithm>
#include <array>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <memory>
#include <glm/vec3.hpp>
#include "block.hpp"
#include "chunkManager.hpp"
#include "../constants.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/quad.hpp"

namespace g = voxel_game::graphics;

namespace voxel_game::world
{
	class World;

	struct Face {
		BlockPos pos;
		g::Direction dir;
	};

	enum class ChunkLod
	{
		FULL = 0,
		HALF = 1,
		QUARTER = 2,
	};

	class Chunk
	{
	private:
		BlockPos m_origin;
		World* m_world;
		BlockTypeId* m_blocks;
		std::array<std::shared_ptr<g::Mesh>, CHUNK_LOD_LEVEL_COUNT> m_meshes = {};
		std::array<std::shared_ptr<g::Mesh>, CHUNK_LOD_LEVEL_COUNT> m_transparentMeshes = {};
		std::array<bool, CHUNK_LOD_LEVEL_COUNT> m_lodBuildQueued = {};
		std::mutex m_mutex;

		bool isBlockInBounds(const BlockPos& blockPos) const;
		bool isFaceVisible(const BlockTypeId& blockTypeId, const Face& face, ChunkManager& chunkManager);
		bool isLodFaceVisible(const BlockTypeId& blockTypeId, const Face& face, int lodScale, ChunkManager& chunkManager);
		Chunk* getNeighbourChunk(graphics::Direction direction, ChunkManager& chunkManager);
		BlockTypeId getRepresentativeBlockType(BlockPos blockPos, int lodScale);
		int getHighestWaterY(BlockPos blockPos, int lodScale);
		BlockTypeId getBlockForLodOcclusion(BlockPos blockPos, ChunkManager& chunkManager);
		void buildMeshForLod(int lodScale, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, std::vector<g::Vertex>& transparentVertices, std::vector<GLuint>& transparentIndices);

		void addFaceToMesh(BlockTypeId blockTypeId, g::Direction direction, BlockPos worldBlockPos, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, glm::vec3 scale = glm::vec3(1.f));

	public:
		Chunk(BlockPos chunkCoord, World* world);
		void updateMesh(ChunkManager& chunkManager);
		void updateMesh(ChunkManager& chunkManager, ChunkLod lod);
		void putBlock(Block block);
		BlockTypeId getBlock(int x, int y, int z);
		BlockTypeId getBlock(BlockPos blockPos);
		BlockPos getOrigin() const;
		BlockPos getChunkCoord();
		std::unique_lock<std::mutex> acquireLock();
		bool hasMesh(ChunkLod lod);
		bool tryQueueMeshBuild(ChunkLod lod);
		std::shared_ptr<g::Mesh> getMesh();
		std::shared_ptr<g::Mesh> getMesh(ChunkLod lod);
		std::shared_ptr<g::Mesh> getTransparentMesh();
		std::shared_ptr<g::Mesh> getTransparentMesh(ChunkLod lod);
	};

	int to1dIndex(int x, int y, int z);
	int to1dIndex(BlockPos pos);
	BlockPos to3dIndex(int i);
	glm::vec3 toVec3(BlockPos blockPos);
}
