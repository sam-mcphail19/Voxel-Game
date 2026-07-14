#include "world.hpp"
#include "../graphics/frustum.hpp"

namespace voxel_game::world
{
	World::World(long seed, g::Shader* shader, Player& player)
		: m_seed(seed), m_chunkManager(ChunkManager()), m_worldGenerator(BiomeBasedWorldGenerator(seed)), m_threadPool(utils::ThreadPool()), m_shader(shader), m_player(player)
	{
		m_threadPool.start();
	}

	World::~World()
	{
		m_threadPool.stop();
	}

	void World::generate()
	{
		const auto start = std::chrono::system_clock::now();
		int queuedChunks = 0;

		for (int x = -CHUNK_RENDER_DISTANCE - 1; x < CHUNK_RENDER_DISTANCE + 1; x++)
		{
			for (int z = -CHUNK_RENDER_DISTANCE - 1; z < CHUNK_RENDER_DISTANCE + 1; z++)
			{
				generateChunkAsync({ x, 0, z });
				queuedChunks++;
			}
		}

		log::info("Queued " + std::to_string(queuedChunks) + " chunks for initial world generation");

		while (!allChunksGenerated())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		const auto end = std::chrono::system_clock::now();
		auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		log::info("World generation took " + std::to_string(durationMs) + "ms");
	}

	DebugInfo World::update()
	{
		PlayerControl input = m_player.getInput();

		RaycastResult blockLookingAt = getBlockLookingAt();
		if (blockLookingAt.block != NULL_BLOCK)
		{
			m_shader->setUniform1i(IS_SELECTED_BLOCK_UNIFORM, 1);
			m_shader->setUniform3i(SELECTED_BLOCK_UNIFORM, blockLookingAt.block.pos.x, blockLookingAt.block.pos.y, blockLookingAt.block.pos.z);

			if (input.mouseOneDown)
			{
				if (m_blockBeingBroken != blockLookingAt.block.pos)
				{
					m_breakBlockProgress = 0;
					m_blockBeingBroken = blockLookingAt.block.pos;
				}

				m_breakBlockProgress++;

				if (m_breakBlockProgress >= 30)
				{
					log::info("Removing: " + blockLookingAt.block.pos);
					removeBlock(blockLookingAt.block.pos);
					m_breakBlockProgress = 0;
				}

				float breakBlockProg = m_breakBlockProgress / 30.f;
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

			if (input.mouseTwoPressed)
			{
				// TODO: block type selection
				Block block = { blockLookingAt.block.pos + g::getNormalI(blockLookingAt.hitFace), BlockTypeId::STONE };
				if (canPlaceBlock(block))
				{
					putBlock(block);
				}
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

		std::vector<BlockPos> chunksToGenerate = getChunksToGenerate();

		for (BlockPos pos : chunksToGenerate)
		{
			generateChunkAsync(pos);
		}

		BlockPos playerPos = toBlockPos(m_player.getPos());

		if (input::isKeyPressed(GLFW_KEY_P))
		{
			log::info("C: " + std::to_string(m_worldGenerator.calcContinentalness(playerPos.x, playerPos.z)));
			log::info("E: " + std::to_string(m_worldGenerator.calcErosion(playerPos.x, playerPos.z)));
			log::info("T: " + std::to_string(m_worldGenerator.calcTemperature(playerPos.x, playerPos.z)));
			log::info("H: " + std::to_string(m_worldGenerator.calcHumidity(playerPos.x, playerPos.z)));

			std::vector<BiomeWeight> weights = m_worldGenerator.buildWeights(playerPos.x, playerPos.z);
			for (BiomeWeight weight : weights)
			{
				log::info(toString(weight.biome->type) + ": " + std::to_string(weight.weight));
			}
			log::info("----------------------------");
		}

		return DebugInfo{
			playerPos.x,
			playerPos.y,
			playerPos.z,
		};
	}

	std::vector<Chunk*> World::getVisibleChunks()
	{
		std::vector<Chunk*> visibleChunks;
		std::array<g::Plane, 6> frustum = g::buildCameraFrustum(m_player.getCamera());
		for (Chunk* chunk : m_chunkManager.getChunks())
		{
			if (chunk->getMesh() != nullptr && m_player.chunkIsVisible(chunk))
			{
				if (g::chunkIntersectsFrustum(chunk, frustum))
				{
					queueVisibleChunkLodBuild(chunk);
				}
				visibleChunks.push_back(chunk);
			}
		}

		std::sort(visibleChunks.begin(), visibleChunks.end(), [&](Chunk* chunk1, Chunk* chunk2)
			{
				return chunkIsFurtherFromPlayer(chunk1, chunk2, m_player);
			}
		);

		return visibleChunks;
	}

	int World::uploadPendingMeshes()
	{
		int uploads = 0;
		for (Chunk* chunk : m_chunkManager.getChunks())
		{
			if (!chunk->hasPendingMeshUpload())
			{
				continue;
			}

			uploads += chunk->uploadPendingMeshes();
		}

		return uploads;
	}

	BlockTypeId World::getBlock(const BlockPos& blockPos)
	{
		BlockPos chunkCoord = getChunkCoord(blockPos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == nullptr)
		{
			return m_worldGenerator.getBlockType(blockPos);
		}

		BlockPos localBlockPos = worldPosToLocalPos(blockPos);
		return chunk->getBlock(localBlockPos);
	}

	void World::removeBlock(const BlockPos& blockPos)
	{
		BlockPos chunkCoord = getChunkCoord(blockPos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == nullptr)
		{
			return;
		}

		BlockPos localBlockPos = worldPosToLocalPos(blockPos);
		chunk->putBlock(Block{ localBlockPos, BlockTypeId::AIR });
		updateChunkMeshAsync(chunk);

		// TODO: refactor to be less repetitive
		if (localBlockPos.x == 0)
		{
			chunk = m_chunkManager.getChunk(chunkCoord - BlockPos{ 1, 0, 0 });
			if (chunk)
			{
				updateChunkMeshAsync(chunk);
			}
		}
		else if (localBlockPos.x == CHUNK_SIZE - 1)
		{
			chunk = m_chunkManager.getChunk(chunkCoord + BlockPos{ 1, 0, 0 });
			if (chunk)
			{
				updateChunkMeshAsync(chunk);
			}
		}

		if (localBlockPos.z == 0)
		{
			chunk = m_chunkManager.getChunk(chunkCoord - BlockPos{ 0, 0, 1 });
			if (chunk)
			{
				updateChunkMeshAsync(chunk);
			}
		}
		else if (localBlockPos.z == CHUNK_SIZE - 1)
		{
			chunk = m_chunkManager.getChunk(chunkCoord + BlockPos{ 0, 0, 1 });
			if (chunk)
			{
				updateChunkMeshAsync(chunk);
			}
		}
	}

	void World::putBlock(const Block& block)
	{
		BlockPos chunkCoord = getChunkCoord(block.pos);
		Chunk* chunk = m_chunkManager.getChunk(chunkCoord);
		if (chunk == nullptr)
		{
			return;
		}

		BlockPos localBlockPos = worldPosToLocalPos(block.pos);
		chunk->putBlock(Block{ localBlockPos, block.type });
		updateChunkMeshAsync(chunk);
	}

	bool World::canPlaceBlock(const Block& block)
	{
		// TODO
		return true;
	}

	BlockPos World::getChunkCoord(BlockPos pos)
	{
		int x = utils::floorDiv(pos.x, CHUNK_SIZE);
		int y = utils::floorDiv(pos.y, CHUNK_HEIGHT);
		int z = utils::floorDiv(pos.z, CHUNK_SIZE);

		y = utils::max(0, y);

		return { x, y, z };
	}

	RaycastResult World::getBlockLookingAt()
	{
		// TODO: test other param values and move to constants.hpp
		return raycast(m_player.getCamera()->getPos(), m_player.getCamera()->viewDir(), 6, 5);
	}

	RaycastResult World::raycast(const glm::vec3 startPos, const glm::vec3 rayDir, int maxDist, int iterationsPerBlock)
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

			if (isSolid(blockTypeId)) {
				g::Direction hitFace = g::Direction::NONE;

				glm::vec3 blockCenter = glm::vec3(blockPos.x + 0.5f, blockPos.y + 0.5f, blockPos.z + 0.5f);
				glm::vec3 hitPoint = pos - blockCenter;

				if (fabs(hitPoint.x) > fabs(hitPoint.y) && fabs(hitPoint.x) > fabs(hitPoint.z)) {
					hitFace = (hitPoint.x > 0) ? g::Direction::RIGHT : g::Direction::LEFT;
				}
				else if (fabs(hitPoint.y) > fabs(hitPoint.x) && fabs(hitPoint.y) > fabs(hitPoint.z)) {
					hitFace = (hitPoint.y > 0) ? g::Direction::TOP : g::Direction::BOTTOM;
				}
				else {
					hitFace = (hitPoint.z > 0) ? g::Direction::FRONT : g::Direction::BACK;
				}

				return { Block{ blockPos, blockTypeId }, hitFace };
			}

			if (isSolid(blockTypeId))
			{
				return { Block{ blockPos, blockTypeId } };
			}

			pos += ray;
		} while (glm::length(pos - startPos) < maxDist && pos.y >= 0);

		return { NULL_BLOCK, g::Direction::NONE };
	}

	void World::updateChunkMesh(Chunk* chunk)
	{
		updateChunkMesh(chunk, ChunkLod::FULL);
	}

	void World::updateChunkMesh(Chunk* chunk, ChunkLod lod)
	{
		chunk->updateMesh(m_chunkManager, lod);
	}

	void World::updateChunkMeshAsync(Chunk* chunk)
	{
		updateChunkMeshAsync(chunk, ChunkLod::FULL);
	}

	void World::updateChunkMeshAsync(Chunk* chunk, ChunkLod lod)
	{
		const std::function<void()>& job = [&, chunk, lod]()
			{
				updateChunkMesh(chunk, lod);
			};
		m_threadPool.queueJob(job);
	}

	void World::generateChunk(BlockPos chunkCoord)
	{
		Chunk* chunk = new Chunk(chunkCoord, this);
		BiomeBasedWorldGenerator* worldGenerator = new BiomeBasedWorldGenerator(m_seed);
		worldGenerator->generateChunkData(*chunk);
		delete worldGenerator;

		m_chunkManager.putChunk(chunkCoord, chunk);

		updateChunkMesh(chunk);
	}

	void World::generateChunkAsync(BlockPos chunkCoord)
	{
		{
			std::unique_lock<std::mutex> lock(m_generatingChunksMutex);
			m_generatingChunks.insert(chunkCoord);
		}

		const std::function<void()>& job = [&, chunkCoord]()
			{
				generateChunk(chunkCoord);

				std::unique_lock<std::mutex> lock(m_generatingChunksMutex);
				if (m_generatingChunks.contains(chunkCoord))
				{
					m_generatingChunks.erase(chunkCoord);
				}
			};
		m_threadPool.queueJob(job);
	}

	std::vector<BlockPos> World::getChunksToGenerate()
	{
		BlockPos currentChunkCoord = getChunkCoord(toBlockPos(m_player.getPos()));
		int worldHeightInChunks = (WORLD_HEIGHT + CHUNK_HEIGHT - 1) / CHUNK_HEIGHT;
		int verticalChunkRenderDistance = 1;
		int yStart = utils::max(currentChunkCoord.y - verticalChunkRenderDistance, 0);
		int yEnd = utils::min(currentChunkCoord.y + verticalChunkRenderDistance, worldHeightInChunks - 1);


		std::vector<BlockPos> result;

		// TODO: Should go from closest to player -> furthest to player
		for (int x = -CHUNK_RENDER_DISTANCE; x < CHUNK_RENDER_DISTANCE; x++)
		{
			for (int z = -CHUNK_RENDER_DISTANCE; z < CHUNK_RENDER_DISTANCE; z++)
			{
				for (int y = yStart; y <= yEnd; y++)
				{
					BlockPos pos = { currentChunkCoord.x + x, y, currentChunkCoord.z + z };
					if (!m_chunkManager.containsChunk(pos))
					{
						std::unique_lock<std::mutex> lock(m_generatingChunksMutex);
						if (!m_generatingChunks.contains(pos))
						{
							result.push_back(pos);
						}
					}
				}
			}
		}

		return result;
	}

	bool World::allChunksGenerated()
	{
		std::unique_lock<std::mutex> lock(m_generatingChunksMutex);
		return m_generatingChunks.empty();
	}

	ChunkLod World::getChunkLodForPlayer(Chunk* chunk, const Player& player)
	{
		BlockPos chunkCenter = chunk->getOrigin() + BlockPos{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };
		glm::vec3 playerPos = player.getPos();
		float x = playerPos.x - chunkCenter.x;
		float z = playerPos.z - chunkCenter.z;
		float distance = std::sqrt(x * x + z * z);

		if (distance >= CHUNK_LOD_2_DISTANCE_IN_BLOCKS)
		{
			return ChunkLod::QUARTER;
		}
		if (distance >= CHUNK_LOD_1_DISTANCE_IN_BLOCKS)
		{
			return ChunkLod::HALF;
		}
		return ChunkLod::FULL;
	}

	void World::queueVisibleChunkLodBuild(Chunk* chunk)
	{
		ChunkLod lod = getChunkLodForPlayer(chunk, m_player);
		if (lod == ChunkLod::FULL)
		{
			return;
		}

		{
			std::unique_lock<std::mutex> lock = chunk->acquireLock();
			if (!chunk->tryQueueMeshBuild(lod))
			{
				return;
			}
		}

		updateChunkMeshAsync(chunk, lod);
	}

	bool World::chunkIsFurtherFromPlayer(Chunk* chunk1, Chunk* chunk2, const Player& player)
	{
		BlockPos chunk1Center = chunk1->getOrigin() + BlockPos{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };
		BlockPos chunk2Center = chunk2->getOrigin() + BlockPos{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };

		float chunk1Dist = glm::length(player.getPos() - toVec3(chunk1Center));
		float chunk2Dist = glm::length(player.getPos() - toVec3(chunk2Center));

		return chunk1Dist > chunk2Dist;
	}

	BlockPos worldPosToLocalPos(const BlockPos& worldPos)
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
