#include "world.hpp"

namespace voxel_game::world
{
	namespace
	{
		const std::array<int, CHUNK_LOD_LEVEL_COUNT> lodScales = { 1, 2, 4 };
		const std::array<const char*, CHUNK_LOD_LEVEL_COUNT> lodNames = { "full", "half", "quarter" };

		int toLodIndex(ChunkLod lod)
		{
			return static_cast<int>(lod);
		}

		long long elapsedMs(std::chrono::system_clock::time_point start)
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now() - start
			).count();
		}

		bool shouldDrawAgainstNeighbour(BlockTypeId blockTypeId, BlockTypeId neighbour)
		{
			if (blockTypeId == BlockTypeId::WATER)
			{
				return isTransparent(neighbour) && neighbour != BlockTypeId::WATER;
			}

			return isTransparent(neighbour);
		}

		g::AtlasTexture getTextureForFace(BlockTypeId blockTypeId, g::Direction direction)
		{
			switch (blockTypeId)
			{
			case BlockTypeId::WATER:
				return g::AtlasTexture::WATER;
			case BlockTypeId::STONE:
				return g::AtlasTexture::STONE;
			case BlockTypeId::DIRT:
				return g::AtlasTexture::DIRT;
			case BlockTypeId::GRASS:
				if (direction == g::Direction::TOP)
				{
					return g::AtlasTexture::GRASS;
				}
				if (direction == g::Direction::BOTTOM)
				{
					return g::AtlasTexture::DIRT;
				}
				return g::AtlasTexture::GRASS_SIDE;
			case BlockTypeId::BEDROCK:
				return g::AtlasTexture::BEDROCK;
			case BlockTypeId::SAND:
				return g::AtlasTexture::SAND;
			case BlockTypeId::GRAVEL:
				return g::AtlasTexture::GRAVEL;
			case BlockTypeId::SANDSTONE:
				if (direction == g::Direction::TOP || direction == g::Direction::BOTTOM)
				{
					return g::AtlasTexture::SANDSTONE;
				}
				return g::AtlasTexture::SANDSTONE_SIDE;
			default:
				return g::AtlasTexture::STONE;
			}
		}
	}

	Chunk::Chunk(BlockPos chunkCoord, World* world)
		: m_world(world), m_blocks(new BlockTypeId[CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT])
	{
		m_origin = { chunkCoord.x * CHUNK_SIZE, chunkCoord.y * CHUNK_HEIGHT, chunkCoord.z * CHUNK_SIZE };
	}

	void Chunk::updateMesh(ChunkManager& chunkManager)
	{
		updateMesh(chunkManager, ChunkLod::FULL);
	}

	void Chunk::updateMesh(ChunkManager& chunkManager, ChunkLod lod)
	{
		auto totalStart = std::chrono::system_clock::now();
		int lodIndex = toLodIndex(lod);
		std::shared_ptr<g::Mesh> newMesh;
		std::shared_ptr<g::Mesh> newTransparentMesh;

		{
			auto lodStart = std::chrono::system_clock::now();
			std::vector<g::Vertex> vertices;
			std::vector<g::Vertex> transparentVertices;
			std::vector<GLuint> indices;
			std::vector<GLuint> transparentIndices;

			auto faceBuildStart = std::chrono::system_clock::now();
			buildMeshForLod(lodScales[lodIndex], chunkManager, vertices, indices, transparentVertices, transparentIndices);
			long long faceBuildMs = elapsedMs(faceBuildStart);

			auto meshCreateStart = std::chrono::system_clock::now();
			newMesh = std::make_shared<graphics::Mesh>(vertices, indices, nullptr, g::loadTextureAtlas());
			newTransparentMesh = std::make_shared<graphics::Mesh>(transparentVertices, transparentIndices, nullptr, g::loadTextureAtlas());
			long long meshCreateMs = elapsedMs(meshCreateStart);

			log::info("Chunk " + getChunkCoord() +
				" LoD " + lodNames[lodIndex] +
				" mesh profile: faceBuild=" + std::to_string(faceBuildMs) + "ms" +
				", meshCreate=" + std::to_string(meshCreateMs) + "ms" +
				", opaqueVerts=" + std::to_string(vertices.size()) +
				", transparentVerts=" + std::to_string(transparentVertices.size()) +
				", indices=" + std::to_string(indices.size() + transparentIndices.size()) +
				", total=" + std::to_string(elapsedMs(lodStart)) + "ms"
			);
		}

		{
			std::unique_lock<std::mutex> lock = acquireLock();
			if (lod == ChunkLod::FULL)
			{
				m_meshes.fill(nullptr);
				m_transparentMeshes.fill(nullptr);
				m_lodBuildQueued.fill(false);
			}
			m_meshes[lodIndex] = std::move(newMesh);
			m_transparentMeshes[lodIndex] = std::move(newTransparentMesh);
			m_lodBuildQueued[lodIndex] = false;
		}

		(void)totalStart;
	}

	void Chunk::buildMeshForLod(int lodScale, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, std::vector<g::Vertex>& transparentVertices, std::vector<GLuint>& transparentIndices)
	{
		for (int z = 0; z < CHUNK_SIZE; z += lodScale)
		{
			for (int y = 0; y < CHUNK_HEIGHT; y += lodScale)
			{
				for (int x = 0; x < CHUNK_SIZE; x += lodScale)
				{
					BlockPos blockPos = { x, y, z };

					glm::vec3 blockPosVec = toVec3(blockPos);
					BlockTypeId blockTypeId = getRepresentativeBlockType(blockPos, lodScale);

					if (blockTypeId == BlockTypeId::AIR)
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

					for (int i = 0; i < 6; i++)
					{
						g::Direction dir = (g::Direction)i;

						if (!(lodScale == 1
							? isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager)
							: isLodFaceVisible(blockTypeId, Face{ blockPos, dir }, lodScale, chunkManager)))
						{
							continue;
						}

						if (blockTypeId == BlockTypeId::WATER)
						{
							addFaceToMesh(blockTypeId, dir, renderBlockPos + m_origin, blockPosVec, transparentVertices, transparentIndices, meshScale);
						}
						else
						{
							addFaceToMesh(blockTypeId, dir, renderBlockPos + m_origin, blockPosVec, vertices, indices, meshScale);
						}
					}
				}
			}
		}
	}

	void Chunk::addFaceToMesh(BlockTypeId blockTypeId, g::Direction direction, BlockPos worldBlockPos, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, glm::vec3 scale)
	{
		for (int j = 0; j < g::Quad::indexCount; j++)
		{
			indices.push_back(g::Quad::indices[j] + vertices.size());
		}

		int vertexPositionIndex = g::Quad::vertexPositionIndexMap.at(direction);
		int uvIndex = g::Quad::uvIndexMap.at(direction);
		glm::vec2 atlasCoords = g::getTextureAtlasCoords(getTextureForFace(blockTypeId, direction));
		float textureSize = g::getTextureAtlasTextureSize();

		for (int j = 0; j < g::Quad::vertexCount; j++)
		{
			glm::vec3 vertexPos = glm::vec3(
				g::Quad::vertexPositions[vertexPositionIndex + j * 3],
				g::Quad::vertexPositions[vertexPositionIndex + j * 3 + 1],
				g::Quad::vertexPositions[vertexPositionIndex + j * 3 + 2]
			);
			glm::vec2 uv = glm::vec2(
				g::Quad::uvs[uvIndex + j * 2] * textureSize + atlasCoords.x,
				g::Quad::uvs[uvIndex + j * 2 + 1] * textureSize + atlasCoords.y
			);

			vertices.push_back(g::Vertex(
				blockPos + vertexPos * scale,
				g::getNormal(direction),
				uv,
				blockTypeId,
				worldBlockPos,
				true
			));
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

	bool Chunk::hasMesh(ChunkLod lod)
	{
		return m_meshes[toLodIndex(lod)] != nullptr;
	}

	bool Chunk::tryQueueMeshBuild(ChunkLod lod)
	{
		int lodIndex = toLodIndex(lod);
		if (lod == ChunkLod::FULL || m_meshes[lodIndex] != nullptr || m_lodBuildQueued[lodIndex])
		{
			return false;
		}

		m_lodBuildQueued[lodIndex] = true;
		return true;
	}

	std::shared_ptr<g::Mesh> Chunk::getMesh(ChunkLod lod)
	{
		std::shared_ptr<g::Mesh> mesh = m_meshes[toLodIndex(lod)];
		return mesh ? mesh : m_meshes[toLodIndex(ChunkLod::FULL)];
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh()
	{
		return getTransparentMesh(ChunkLod::FULL);
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh(ChunkLod lod)
	{
		std::shared_ptr<g::Mesh> mesh = m_transparentMeshes[toLodIndex(lod)];
		return mesh ? mesh : m_transparentMeshes[toLodIndex(ChunkLod::FULL)];
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
