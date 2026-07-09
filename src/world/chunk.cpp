#include "world.hpp"

namespace voxel_game::world
{
	namespace
	{
		const std::array<int, CHUNK_LOD_LEVEL_COUNT> lodScales = { 1, 2, 4 };

		int toLodIndex(ChunkLod lod)
		{
			return static_cast<int>(lod);
		}

		bool shouldDrawAgainstNeighbour(BlockTypeId blockTypeId, BlockTypeId neighbour)
		{
			if (blockTypeId == BlockTypeId::WATER)
			{
				return isTransparent(neighbour) && neighbour != BlockTypeId::WATER;
			}

			return isTransparent(neighbour);
		}
	}

	Chunk::Chunk(BlockPos chunkCoord, World* world)
		: m_world(world), m_blocks(new BlockTypeId[CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT])
	{
		m_origin = { chunkCoord.x * CHUNK_SIZE, chunkCoord.y * CHUNK_HEIGHT, chunkCoord.z * CHUNK_SIZE };
	}

	void Chunk::updateMesh(ChunkManager& chunkManager)
	{
		std::array<std::shared_ptr<g::Mesh>, CHUNK_LOD_LEVEL_COUNT> newMeshes;
		std::array<std::shared_ptr<g::Mesh>, CHUNK_LOD_LEVEL_COUNT> newTransparentMeshes;

		for (int lod = 0; lod < CHUNK_LOD_LEVEL_COUNT; lod++)
		{
			std::vector<g::Vertex> vertices;
			std::vector<g::Vertex> transparentVertices;
			std::vector<GLuint> indices;
			std::vector<GLuint> transparentIndices;

			buildMeshForLod(lodScales[lod], chunkManager, vertices, indices, transparentVertices, transparentIndices);

			newMeshes[lod] = std::make_shared<graphics::Mesh>(vertices, indices, nullptr, g::loadTextureAtlas());
			newTransparentMeshes[lod] = std::make_shared<graphics::Mesh>(transparentVertices, transparentIndices, nullptr, g::loadTextureAtlas());
		}

		{
			std::unique_lock<std::mutex> lock = acquireLock();
			m_meshes = std::move(newMeshes);
			m_transparentMeshes = std::move(newTransparentMeshes);
		}
	}

	void Chunk::buildMeshForLod(int lodScale, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, std::vector<g::Vertex>& transparentVertices, std::vector<GLuint>& transparentIndices)
	{
		for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
		{
			BlockPos blockPos = to3dIndex(i);
			if (blockPos.x % lodScale != 0 || blockPos.y % lodScale != 0 || blockPos.z % lodScale != 0)
			{
				continue;
			}

			glm::vec3 blockPosVec = toVec3(blockPos);
			BlockTypeId blockTypeId = getRepresentativeBlockType(blockPos, lodScale);

			if (blockTypeId == BlockTypeId::AIR)
			{
				continue;
			}

			bool anyFaceVisible = false;
			for (int i = 0; i < 6; i++)
			{
				g::Direction dir = (g::Direction)i;
				if (lodScale == 1
					? isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager)
					: isLodFaceVisible(blockTypeId, Face{ blockPos, dir }, lodScale, chunkManager))
				{
					anyFaceVisible = true;
					break;
				}
			}

			if (!anyFaceVisible)
			{
				continue;
			}

			BlockPos renderBlockPos = blockPos;
			glm::vec3 meshScale(lodScale);
			if (blockTypeId == BlockTypeId::WATER && lodScale > 1)
			{
				renderBlockPos.y = getHighestWaterY(blockPos, lodScale);
				meshScale.y = 1.f;
				blockPosVec = toVec3(renderBlockPos);
			}

			const BlockType& blockType = BlockType::get(blockTypeId);
			Cube cube = blockType.getMeshConstructor()(Block{ renderBlockPos + m_origin, blockTypeId });

			for (int i = 0; i < 6; i++)
			{
				g::Direction dir = (g::Direction)i;

				if (!(lodScale == 1
					? isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager)
					: isLodFaceVisible(blockTypeId, Face{ blockPos, dir }, lodScale, chunkManager)))
				{
					continue;
				}

				g::Quad* face = cube.getFace(dir);

				if (blockTypeId == BlockTypeId::WATER)
				{
					addFaceToMesh(face, blockPosVec, transparentVertices, transparentIndices, meshScale);
				}
				else
				{
					addFaceToMesh(face, blockPosVec, vertices, indices, meshScale);
				}
			}
		}
	}

	void Chunk::addFaceToMesh(g::Quad* face, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, glm::vec3 scale)
	{
		for (int j = 0; j < g::Quad::indexCount; j++)
		{
			indices.push_back(g::Quad::indices[j] + vertices.size());
		}
		std::vector<g::Vertex> faceVertices = *face->getVertices();
		for (int j = 0; j < g::Quad::vertexCount; j++)
		{
			g::Vertex vert = faceVertices[j];
			vert.m_position = blockPos + vert.m_position * scale;
			vertices.push_back(vert);
		}
	}

	void Chunk::putBlock(Block block)
	{
		m_blocks[to1dIndex(block.pos)] = block.type;
	}

	BlockTypeId Chunk::getBlock(int x, int y, int z)
	{
		return m_blocks[to1dIndex(x, y, z)];
	}

	BlockTypeId Chunk::getBlock(BlockPos blockPos)
	{
		return getBlock(blockPos.x, blockPos.y, blockPos.z);
	}

	BlockPos Chunk::getOrigin() const
	{
		return m_origin;
	}

	BlockPos Chunk::getChunkCoord()
	{
		return { m_origin.x / CHUNK_SIZE, m_origin.y / CHUNK_HEIGHT, m_origin.z / CHUNK_SIZE };
	}

	std::unique_lock<std::mutex> Chunk::acquireLock()
	{
		return std::unique_lock<std::mutex>(m_mutex);
	}

	std::shared_ptr<g::Mesh> Chunk::getMesh()
	{
		return getMesh(ChunkLod::FULL);
	}

	std::shared_ptr<g::Mesh> Chunk::getMesh(ChunkLod lod)
	{
		return m_meshes[toLodIndex(lod)];
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh()
	{
		return getTransparentMesh(ChunkLod::FULL);
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh(ChunkLod lod)
	{
		return m_transparentMeshes[toLodIndex(lod)];
	}

	int to1dIndex(int x, int y, int z)
	{
		return x + CHUNK_SIZE * y + z * CHUNK_SIZE_TIMES_HEIGHT;
	}

	int to1dIndex(BlockPos pos)
	{
		return to1dIndex(pos.x, pos.y, pos.z);
	}

	BlockPos to3dIndex(int i)
	{
		int z = i / CHUNK_SIZE_TIMES_HEIGHT;
		i -= (z * CHUNK_SIZE_TIMES_HEIGHT);
		int y = i / CHUNK_SIZE;
		int x = i % CHUNK_SIZE;

		return { x, y, z };
	}

	glm::vec3 toVec3(BlockPos blockPos)
	{
		return glm::vec3(blockPos.x, blockPos.y, blockPos.z);
	}

	bool Chunk::isBlockInBounds(const BlockPos& blockPos) const {
		return blockPos.x >= 0 && blockPos.x < CHUNK_SIZE &&
			blockPos.z >= 0 && blockPos.z < CHUNK_SIZE;
	}

	bool Chunk::isFaceVisible(const BlockTypeId& blockTypeId, const Face& face, ChunkManager& chunkManager)
	{
		BlockPos neighbourPos = face.pos + toBlockPos(getNormal(face.dir));
		BlockTypeId neighbour;

		if (neighbourPos.y < 0 || neighbourPos.y >= CHUNK_HEIGHT)
		{
			return true;
		}
		if (isBlockInBounds(neighbourPos))
		{
			neighbour = getBlock(neighbourPos);
		}
		else
		{
			Chunk* neighbourChunk = getNeighbourChunk(face.dir, chunkManager);

			if (neighbourChunk == nullptr)
			{
				neighbour = m_world->getBlock(m_origin + neighbourPos);
			}
			else 
			{
				// TODO: Should we just call world.getBlock in this case too?
				BlockPos neighbourLocalPos = worldPosToLocalPos(m_origin + neighbourPos);
				neighbour = neighbourChunk->getBlock(neighbourLocalPos);
			}
		}

		if (blockTypeId == BlockTypeId::WATER)
		{
			return isTransparent(neighbour) && neighbour != BlockTypeId::WATER;
		}

		return isTransparent(neighbour);
	}

	bool Chunk::isLodFaceVisible(const BlockTypeId& blockTypeId, const Face& face, int lodScale, ChunkManager& chunkManager)
	{
		BlockPos start = face.pos;
		BlockPos end = face.pos + BlockPos{ lodScale, lodScale, lodScale };
		if (blockTypeId == BlockTypeId::WATER && lodScale > 1)
		{
			start.y = getHighestWaterY(face.pos, lodScale);
			end.y = start.y + 1;
		}

		switch (face.dir)
		{
		case g::Direction::FRONT:
			start.z = face.pos.z + lodScale;
			end.z = start.z + 1;
			break;
		case g::Direction::BACK:
			start.z = face.pos.z - 1;
			end.z = start.z + 1;
			break;
		case g::Direction::RIGHT:
			start.x = face.pos.x + lodScale;
			end.x = start.x + 1;
			break;
		case g::Direction::LEFT:
			start.x = face.pos.x - 1;
			end.x = start.x + 1;
			break;
		case g::Direction::TOP:
			start.y = face.pos.y + lodScale;
			end.y = start.y + 1;
			break;
		case g::Direction::BOTTOM:
			start.y = face.pos.y - 1;
			end.y = start.y + 1;
			break;
		default:
			return true;
		}

		bool hasWater = false;
		bool hasVisibleTransparent = false;
		for (int x = start.x; x < end.x; x++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int z = start.z; z < end.z; z++)
				{
					BlockTypeId neighbour = getBlockForLodOcclusion(BlockPos{ x, y, z }, chunkManager);
					hasWater = hasWater || neighbour == BlockTypeId::WATER;
					hasVisibleTransparent = hasVisibleTransparent || shouldDrawAgainstNeighbour(blockTypeId, neighbour);
					if (blockTypeId != BlockTypeId::WATER && hasVisibleTransparent)
					{
						return true;
					}
				}
			}
		}

		if (blockTypeId == BlockTypeId::WATER)
		{
			if (face.dir == g::Direction::BOTTOM)
			{
				return false;
			}
			if (face.dir == g::Direction::TOP)
			{
				return hasVisibleTransparent;
			}

			return !hasWater && hasVisibleTransparent;
		}

		return hasVisibleTransparent;
	}

	Chunk* Chunk::getNeighbourChunk(graphics::Direction direction, ChunkManager& chunkManager)
	{
		BlockPos chunkCoord = getChunkCoord() + getNormalI(direction);
		return chunkManager.getChunk(chunkCoord);
	}

	BlockTypeId Chunk::getRepresentativeBlockType(BlockPos blockPos, int lodScale)
	{
		bool hasWater = false;
		int maxX = std::min(blockPos.x + lodScale, CHUNK_SIZE);
		int maxY = std::min(blockPos.y + lodScale, CHUNK_HEIGHT);
		int maxZ = std::min(blockPos.z + lodScale, CHUNK_SIZE);

		for (int y = maxY - 1; y >= blockPos.y; y--)
		{
			for (int x = blockPos.x; x < maxX; x++)
			{
				for (int z = blockPos.z; z < maxZ; z++)
				{
					BlockTypeId blockTypeId = getBlock(x, y, z);
					if (blockTypeId == BlockTypeId::WATER)
					{
						hasWater = true;
					}
					else if (isSolid(blockTypeId))
					{
						return blockTypeId;
					}
				}
			}
		}

		return hasWater ? BlockTypeId::WATER : BlockTypeId::AIR;
	}

	int Chunk::getHighestWaterY(BlockPos blockPos, int lodScale)
	{
		int maxX = std::min(blockPos.x + lodScale, CHUNK_SIZE);
		int maxY = std::min(blockPos.y + lodScale, CHUNK_HEIGHT);
		int maxZ = std::min(blockPos.z + lodScale, CHUNK_SIZE);

		for (int y = maxY - 1; y >= blockPos.y; y--)
		{
			for (int x = blockPos.x; x < maxX; x++)
			{
				for (int z = blockPos.z; z < maxZ; z++)
				{
					if (getBlock(x, y, z) == BlockTypeId::WATER)
					{
						return y;
					}
				}
			}
		}

		return blockPos.y;
	}

	BlockTypeId Chunk::getBlockForLodOcclusion(BlockPos blockPos, ChunkManager& chunkManager)
	{
		if (blockPos.y < 0 || blockPos.y >= CHUNK_HEIGHT)
		{
			return BlockTypeId::AIR;
		}

		if (isBlockInBounds(blockPos))
		{
			return getBlock(blockPos);
		}

		BlockPos worldPos = m_origin + blockPos;
		BlockPos neighbourChunkCoord = {
			utils::floorDiv(worldPos.x, CHUNK_SIZE),
			utils::floorDiv(worldPos.y, CHUNK_HEIGHT),
			utils::floorDiv(worldPos.z, CHUNK_SIZE)
		};
		Chunk* neighbourChunk = chunkManager.getChunk(neighbourChunkCoord);
		if (neighbourChunk != nullptr)
		{
			return neighbourChunk->getBlock(worldPosToLocalPos(worldPos));
		}

		return m_world->getBlock(worldPos);
	}
}
