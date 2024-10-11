#include "world.hpp"

namespace voxel_game::world
{
	World::World(WorldGenerator& worldGenerator, g::Shader* shader, Player& player)
		: m_chunkManager(ChunkManager()), m_worldGenerator(worldGenerator), m_threadPool(utils::ThreadPool()), m_shader(shader), m_player(player)
	{
		m_threadPool.start();
	}

	World::~World()
	{
		m_threadPool.stop();
	}

	void World::generate()
	{
		for (int y = 0; y < WORLD_HEIGHT / CHUNK_HEIGHT; y++)
		{
			generateChunk({ 0, y, 0 });
			generateChunk({ 1, y, 0 });
		}
	}

	void World::update()
	{
		PlayerControl input = m_player.getInput();

		BlockPos currentChunkCoord = getChunkCoord(toBlockPos(m_player.getPos()));
		int yStart = utils::max(currentChunkCoord.y - CHUNK_RENDER_DISTANCE, 0);
		int yEnd = utils::min(currentChunkCoord.y + CHUNK_RENDER_DISTANCE, WORLD_HEIGHT);

		for (int x = -CHUNK_RENDER_DISTANCE; x < CHUNK_RENDER_DISTANCE; x++)
		{
			for (int z = -CHUNK_RENDER_DISTANCE; z < CHUNK_RENDER_DISTANCE; z++)
			{
				BlockPos pos = currentChunkCoord + BlockPos{ x, -currentChunkCoord.y, z };
				if (!m_chunkManager.containsChunk(pos))
				{
					// TODO: Sort this by closest to player -> furthest to player
					//generateChunk(pos);
				}
			}
		}

		Block blockLookingAt = getBlockLookingAt();
		if (blockLookingAt != NULL_BLOCK)
		{
			m_shader->setUniform1i(IS_SELECTED_BLOCK_UNIFORM, 1);
			m_shader->setUniform3i(SELECTED_BLOCK_UNIFORM, blockLookingAt.pos.x, blockLookingAt.pos.y, blockLookingAt.pos.z);

			if (input.mouseOneDown)
			{
				if (m_blockBeingBroken != blockLookingAt.pos)
				{
					m_breakBlockProgress = 0;
					m_blockBeingBroken = blockLookingAt.pos;
				}

				m_breakBlockProgress++;

				if (m_breakBlockProgress >= 30)
				{
					removeBlock(blockLookingAt.pos);
					m_breakBlockProgress = 0;
				}

				float breakBlockProg = (float)m_breakBlockProgress / 30;
				m_shader->setUniform1f(BLOCK_BREAK_PROG_UNIFORM, breakBlockProg);
			}
			else
			{
				if (m_breakBlockProgress > 0)
				{
					m_shader->setUniform1f(BLOCK_BREAK_PROG_UNIFORM, 0);
				}

				m_breakBlockProgress = 0;
			}

			if (input.mouseTwoDown)
			{
				// need to get the side of the block being looked at and place it there
				// putBlock(Block{blockLookingAt.pos,});;
			}
		}
		else
		{
			m_shader->setUniform1i(IS_SELECTED_BLOCK_UNIFORM, 0);
		}

		if (m_player.isAffectedByGravity())
		{
			input.movement.y -= GRAVITY;
		}

		m_player.setPos(m_player.getPos() + input.movement);
	}

	std::vector<Chunk*> World::getVisibleChunks()
	{
		std::vector<Chunk*> visibleChunks;
		for (Chunk* chunk : m_chunkManager.getChunks())
		{
			if (chunk->getMesh() != NULL && m_player.chunkIsVisible(chunk))
			{
				visibleChunks.push_back(chunk);
			}
		}
		return visibleChunks;
	}

	BlockTypeId World::getBlock(BlockPos blockPos)
	{
		BlockPos chunkCoord = getChunkCoord(blockPos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == NULL)
		{
			return BlockTypeId::NONE;
		}

		BlockPos localBlockPos = worldPosToLocalPos(blockPos);
		return chunk->getBlock(localBlockPos);
	}

	void World::removeBlock(BlockPos blockPos)
	{
		BlockPos chunkCoord = getChunkCoord(blockPos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == NULL)
		{
			return;
		}

		BlockPos localBlockPos = worldPosToLocalPos(blockPos);
		chunk->putBlock(Block{ localBlockPos, BlockTypeId::AIR });
		updateChunkMesh(chunk);
	}

	void World::putBlock(Block block)
	{
		BlockPos chunkCoord = getChunkCoord(block.pos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == NULL)
		{
			return;
		}

		BlockPos localBlockPos = worldPosToLocalPos(block.pos);
		chunk->putBlock(Block{ localBlockPos, block.type });
		updateChunkMesh(chunk);
	}

	BlockPos World::getChunkCoord(BlockPos pos)
	{
		int x = utils::floorDiv(pos.x, CHUNK_SIZE);
		int y = utils::floorDiv(pos.y, CHUNK_HEIGHT);
		int z = utils::floorDiv(pos.z, CHUNK_SIZE);

		y = utils::max(0, y);

		return { x, y, z };
	}

	Block World::getBlockLookingAt()
	{
		// TODO: test other param values and move to constants.hpp
		return raycast(m_player.getCamera()->getPos(), m_player.getCamera()->viewDir(), 6, 5);
	}

	Block World::raycast(const glm::vec3 startPos, const glm::vec3 rayDir, int maxDist, int iterationsPerBlock)
	{
		glm::vec3 ray = glm::normalize(rayDir) * (1.f / iterationsPerBlock);

		glm::vec3 pos = glm::vec3(startPos);

		std::set<BlockPos> checkedBlocks;

		do
		{
			BlockPos blockPos{ (int)pos.x - (pos.x < 0), (int)pos.y - (pos.y < 0), (int)pos.z - (pos.z < 0) };

			if (checkedBlocks.find(blockPos) != checkedBlocks.end())
			{
				pos += ray;
				continue;
			}

			checkedBlocks.insert(blockPos);

			BlockTypeId blockTypeId = getBlock(blockPos);
			if (isSolid(blockTypeId))
			{
				return Block{ blockPos, blockTypeId };
			}

			pos += ray;
		} while (glm::length(pos - startPos) < maxDist && pos.y >= 0);

		return NULL_BLOCK;
	}

	void World::updateChunkMesh(Chunk* chunk)
	{
		const std::function<void()>& job = [generator = ChunkGenerator(*chunk, m_chunkManager)]() mutable
			{
				generator.run();
			};
		m_threadPool.queueJob(job);
		//ChunkGenerator(*chunk, m_chunkManager).run();
	}

	void World::generateChunk(BlockPos chunkCoord)
	{
		log::info("Generating chunk data for chunk at " + chunkCoord);

		Chunk* chunk = new Chunk(chunkCoord, this);
		m_worldGenerator.generateChunkData(*chunk);

		log::info("Finished generating chunk data for chunk at " + chunkCoord);

		m_chunkManager.putChunk(chunkCoord, chunk);

		updateChunkMesh(chunk);
	}

	BlockPos worldPosToLocalPos(const BlockPos worldPos)
	{
		int x = worldPos.x;
		int z = worldPos.z;
		while (x < 0)
			x += CHUNK_SIZE;
		while (z < 0)
			z += CHUNK_SIZE;

		x %= CHUNK_SIZE;
		z %= CHUNK_SIZE;

		return BlockPos{ x, worldPos.y % CHUNK_HEIGHT, z };
	}
}