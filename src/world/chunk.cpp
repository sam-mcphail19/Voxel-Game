#include "world.hpp"

namespace voxel_game::world
{
	Chunk::Chunk(BlockPos chunkCoord, World* world)
		: m_blocks(new BlockTypeId[CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT]), m_world(world)
	{
		m_origin = { chunkCoord.x * CHUNK_SIZE, chunkCoord.y * CHUNK_HEIGHT, chunkCoord.z * CHUNK_SIZE };
	}

	void Chunk::updateMesh(ChunkManager& chunkManager)
	{
		std::vector<g::Vertex> vertices;
		std::vector<g::Vertex> transparentVertices;
		std::vector<GLuint> indices;
		std::vector<GLuint> transparentIndices;

		glm::vec3 origin = toVec3(m_origin);

		for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
		{
			BlockPos blockPos = to3dIndex(i);
			glm::vec3 blockPosVec = toVec3(blockPos);
			BlockTypeId blockTypeId = getBlock(blockPos);

			if (blockTypeId == BlockTypeId::AIR)
			{
				continue;
			}

			bool anyFaceVisible = false;
			for (int i = 0; i < 6; i++)
			{
				g::Direction dir = (g::Direction)i;
				if (isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager))
				{
					anyFaceVisible = true;
					break;
				}
			}

			if (!anyFaceVisible)
			{
				continue;
			}

			const BlockType& blockType = BlockType::get(blockTypeId);
			Cube cube = blockType.getMeshConstructor()(Block{ blockPos + m_origin, blockTypeId });

			for (int i = 0; i < 6; i++)
			{
				g::Direction dir = (g::Direction)i;

				if (!isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager))
				{
					continue;
				}

				g::Quad* face = cube.getFace(dir);

				if (blockTypeId == BlockTypeId::WATER)
				{
					addFaceToMesh(face, blockPosVec, transparentVertices, transparentIndices);
				}
				else
				{
					addFaceToMesh(face, blockPosVec, vertices, indices);
				}
			}
		}

		g::Mesh* newMesh = new graphics::Mesh(vertices, indices, nullptr, g::loadTextureAtlas());
		g::Mesh* newTransparentMesh = new graphics::Mesh(transparentVertices, transparentIndices, nullptr, g::loadTextureAtlas());

		std::unique_lock<std::mutex> lock = acquireLock();

		std::swap(m_mesh, newMesh);
		std::swap(m_transparentMesh, newTransparentMesh);

		delete newMesh;
		newMesh = nullptr;
		delete newTransparentMesh;
		newTransparentMesh = nullptr;
	}

	void Chunk::addFaceToMesh(g::Quad* face, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices)
	{
		for (int j = 0; j < g::Quad::vertexCount; j++)
		{
			indices.push_back(g::Quad::indices[j] + vertices.size());
		}
		std::vector<g::Vertex> faceVertices = *face->getVertices();
		for (int j = 0; j < g::Quad::vertexCount; j++)
		{
			g::Vertex vert = faceVertices[j];
			vert.m_position += blockPos;
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

	graphics::Mesh* Chunk::getMesh()
	{
		return m_mesh;
	}

	graphics::Mesh* Chunk::getTransparentMesh()
	{
		return m_transparentMesh;
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

	bool Chunk::isFaceVisible(const BlockTypeId& blockTypeId, const Face& face, ChunkManager& chunkManager)
	{
		BlockPos neighbourPos = face.pos + toBlockPos(getNormal(face.dir));
		bool neighbourInThisChunk = neighbourPos.x >= 0 &&
			neighbourPos.x < CHUNK_SIZE &&
			neighbourPos.z >= 0 &&
			neighbourPos.z < CHUNK_SIZE &&
			neighbourPos.y >= 0 &&
			neighbourPos.y < CHUNK_HEIGHT;

		BlockTypeId neighbour;

		if (neighbourInThisChunk)
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

	Chunk* Chunk::getNeighbourChunk(graphics::Direction direction, ChunkManager& chunkManager)
	{
		BlockPos chunkCoord = getChunkCoord() + getNormalI(direction);
		return chunkManager.getChunk(chunkCoord);
	}
}