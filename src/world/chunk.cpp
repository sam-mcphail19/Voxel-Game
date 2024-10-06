#include "world.hpp"

namespace voxel_game::world
{
	Chunk::Chunk(BlockPos chunkCoord, World *world)
		: m_blocks(new BlockTypeId[CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT]), m_world(world)
	{
		m_origin = {chunkCoord.x * CHUNK_SIZE, chunkCoord.y * CHUNK_HEIGHT, chunkCoord.z * CHUNK_SIZE};
	}

	void Chunk::updateMesh(ChunkManager &chunkManager)
	{
		std::vector<g::Vertex> vertices;
		std::vector<GLuint> indices;

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

			physics::Transform transform(blockPosVec + origin, glm::mat4(1), glm::vec3(1));
			const BlockType &blockType = BlockType::get(blockTypeId);
			Cube cube = blockType.getMeshConstructor()(Block{blockPos + m_origin, blockTypeId}, transform);
			std::map<g::Direction, g::Quad *> faces = *cube.getFaces();

			for (auto face = faces.begin(); face != faces.end(); face++)
			{
				if (!isFaceVisible(blockPos, face->first, chunkManager))
				{
					continue;
				}

				std::vector<GLuint> faceIndices = face->second->getIndices();
				for (int j = 0; j < face->second->getVertexCount(); j++)
				{
					indices.push_back(faceIndices[j] + vertices.size());
				}
				std::vector<graphics::Vertex> faceVertices = face->second->getVertices();
				for (int j = 0; j < face->second->getVertexCount(); j++)
				{
					g::Vertex vert = faceVertices[j];
					vert.m_position += blockPosVec;
					vertices.push_back(vert);
				}
			}
		}

		if (m_mesh)
		{
			delete m_mesh;
			m_mesh = NULL;
		}

		// TODO:  use smart pointers for m_mesh to handle automatic memory management
		physics::Transform transform(origin, glm::mat4(1), glm::vec3(1));
		m_mesh = new graphics::Mesh(vertices, indices, transform, g::loadTextureAtlas());
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

	BlockPos Chunk::getOrigin()
	{
		return m_origin;
	}

	BlockPos Chunk::getChunkCoord()
	{
		return {m_origin.x / CHUNK_SIZE, m_origin.y / CHUNK_HEIGHT, m_origin.z / CHUNK_SIZE};
	}

	graphics::Mesh *Chunk::getMesh()
	{
		return m_mesh;
	}

	int to1dIndex(int x, int y, int z)
	{
		return x + CHUNK_SIZE * y + z * CHUNK_SIZE * CHUNK_HEIGHT;
	}

	int to1dIndex(BlockPos pos)
	{
		return to1dIndex(pos.x, pos.y, pos.z);
	}

	BlockPos to3dIndex(int i)
	{
		int z = i / (CHUNK_SIZE * CHUNK_HEIGHT);
		i -= (z * CHUNK_SIZE * CHUNK_HEIGHT);
		int y = i / CHUNK_SIZE;
		int x = i % CHUNK_SIZE;

		return {x, y, z};
	}

	glm::vec3 toVec3(BlockPos blockPos)
	{
		return glm::vec3(blockPos.x, blockPos.y, blockPos.z);
	}

	bool Chunk::isFaceVisible(BlockPos pos, graphics::Direction direction, ChunkManager &chunkManager)
	{
		BlockPos neighbourPos = pos + toBlockPos(getNormal(direction));
		bool neighbourInThisChunk = neighbourPos.x >= 0 &&
									neighbourPos.x < CHUNK_SIZE &&
									neighbourPos.z >= 0 &&
									neighbourPos.z < CHUNK_SIZE &&
									neighbourPos.y >= 0 &&
									neighbourPos.y < CHUNK_HEIGHT;

		if (neighbourInThisChunk)
		{
			BlockTypeId neighbour = getBlock(neighbourPos);
			return neighbour == BlockTypeId::NONE || neighbour == BlockTypeId::AIR;
		}

		Chunk *neighbourChunk = getNeighbourChunk(direction, chunkManager);

		if (neighbourChunk == NULL)
		{
			return true;
		}

		neighbourPos.x = (CHUNK_SIZE + (neighbourPos.x % CHUNK_SIZE)) % CHUNK_SIZE;
		neighbourPos.y %= CHUNK_HEIGHT;
		neighbourPos.z = (CHUNK_SIZE + (neighbourPos.z % CHUNK_SIZE)) % CHUNK_SIZE;

		BlockTypeId neighbour = neighbourChunk->getBlock(neighbourPos);

		return neighbour == BlockTypeId::NONE || neighbour == BlockTypeId::AIR;
	}

	Chunk *Chunk::getNeighbourChunk(graphics::Direction direction, ChunkManager &chunkManager)
	{
		BlockPos chunkCoord = getChunkCoord() + toBlockPos(getNormal(direction));
		return chunkManager.getChunk(chunkCoord);
	}
}