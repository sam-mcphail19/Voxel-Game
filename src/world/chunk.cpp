#include "world.hpp"
#include "worldProfiler.hpp"

namespace voxel_game::world
{
	namespace
	{
		const std::array<int, CHUNK_LOD_LEVEL_COUNT> lodScales = { 1, 2, 4 };
		constexpr uint8_t BITS_PER_BLOCK = 5;
		constexpr uint16_t BLOCK_ID_MASK = (1u << BITS_PER_BLOCK) - 1u;

		static_assert(static_cast<uint8_t>(BlockTypeId::TERRACOTTA) <= BLOCK_ID_MASK,
			"Packed chunk storage supports at most 32 block types");

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

		size_t representativeIndex(BlockPos blockPos, int lodScale)
		{
			const int xCells = CHUNK_SIZE / lodScale;
			const int yCells = CHUNK_HEIGHT / lodScale;
			const int cellX = blockPos.x / lodScale;
			const int cellY = blockPos.y / lodScale;
			const int cellZ = blockPos.z / lodScale;
			return static_cast<size_t>(cellX + xCells * (cellY + yCells * cellZ));
		}

		g::AtlasTexture getTextureForFace(BlockTypeId blockTypeId, g::Direction direction, int lodScale)
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
				if (lodScale > 1)
				{
					// Repeating GRASS_SIDE vertically makes every texel-sized
					// section look like another grass-topped block at coarse LODs.
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
			case BlockTypeId::MARSH_GRASS:
				if (direction == g::Direction::TOP)
				{
					return g::AtlasTexture::MARSH_GRASS;
				}
				return g::AtlasTexture::MUD;
			case BlockTypeId::MUD:
				return g::AtlasTexture::MUD;
			case BlockTypeId::RED_SANDSTONE:
				return g::AtlasTexture::RED_SANDSTONE;
			case BlockTypeId::SNOW:
				return g::AtlasTexture::SNOW;
			case BlockTypeId::ICE:
				return g::AtlasTexture::ICE;
			case BlockTypeId::PODZOL:
				return g::AtlasTexture::PODZOL;
			case BlockTypeId::DRY_GRASS:
				return g::AtlasTexture::DRY_GRASS;
			case BlockTypeId::RAINFOREST_GRASS:
				return g::AtlasTexture::RAINFOREST_GRASS;
			case BlockTypeId::BASALT:
				return g::AtlasTexture::BASALT;
			case BlockTypeId::LAVA:
				return g::AtlasTexture::LAVA;
			case BlockTypeId::SALT:
				return g::AtlasTexture::SALT;
			case BlockTypeId::TERRACOTTA:
				return g::AtlasTexture::TERRACOTTA;
			default:
				return g::AtlasTexture::STONE;
			}
		}
	}

	Chunk::Chunk(BlockPos chunkCoord, World* world)
		: m_world(world),
		  m_blocks((CHUNK_BLOCK_COUNT * BITS_PER_BLOCK + 7) / 8 + 1, 0)
	{
		m_origin = { chunkCoord.x * CHUNK_SIZE, chunkCoord.y * CHUNK_HEIGHT, chunkCoord.z * CHUNK_SIZE };
	}

	Chunk::~Chunk()
	{
	}

	void Chunk::updateMesh(ChunkManager& chunkManager)
	{
		updateMesh(chunkManager, ChunkLod::FULL);
	}

	void Chunk::updateMesh(ChunkManager& chunkManager, ChunkLod lod)
	{
		ScopedProfileCounterBatch counterBatch;
		int lodIndex = toLodIndex(lod);
		switch (lod)
		{
		case ChunkLod::FULL:
			WorldProfiler::instance().increment(ProfileCounter::FullLodBuilds);
			break;
		case ChunkLod::HALF:
			WorldProfiler::instance().increment(ProfileCounter::HalfLodBuilds);
			break;
		case ChunkLod::QUARTER:
			WorldProfiler::instance().increment(ProfileCounter::QuarterLodBuilds);
			break;
		}
		std::shared_ptr<g::Mesh> newMesh;
		std::shared_ptr<g::Mesh> newTransparentMesh;

		{
			std::vector<g::Vertex> vertices;
			std::vector<g::Vertex> transparentVertices;
			std::vector<GLuint> indices;
			std::vector<GLuint> transparentIndices;

			buildMeshForLod(lodScales[lodIndex], chunkManager, vertices, indices, transparentVertices, transparentIndices);

			if (!vertices.empty())
			{
				newMesh = std::make_shared<graphics::Mesh>(vertices, indices, nullptr, g::loadTextureAtlas());
			}
			if (!transparentVertices.empty())
			{
				newTransparentMesh = std::make_shared<graphics::Mesh>(transparentVertices, transparentIndices, nullptr, g::loadTextureAtlas());
			}
		}

		{
			std::unique_lock<std::mutex> lock = acquireLock();
			if (lod == ChunkLod::FULL)
			{
				m_pendingMeshes.fill(nullptr);
				m_pendingTransparentMeshes.fill(nullptr);
				m_pendingMeshReady.fill(false);
				m_lodBuildQueued.fill(false);
			}
			m_pendingMeshes[lodIndex] = std::move(newMesh);
			m_pendingTransparentMeshes[lodIndex] = std::move(newTransparentMesh);
			m_pendingMeshReady[lodIndex] = true;
			m_lodBuildQueued[lodIndex] = false;
		}
	}

	void Chunk::buildMeshForLod(int lodScale, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, std::vector<g::Vertex>& transparentVertices, std::vector<GLuint>& transparentIndices)
	{
		const int xCells = CHUNK_SIZE / lodScale;
		const int yCells = CHUNK_HEIGHT / lodScale;
		const int zCells = CHUNK_SIZE / lodScale;
		std::vector<BlockTypeId> representatives(
			static_cast<size_t>(xCells * yCells * zCells), BlockTypeId::AIR);
		{
			ScopedProfileStage timer(ProfileStage::RepresentativeGridBuild);
			for (int cellZ = 0; cellZ < zCells; ++cellZ)
			{
				for (int cellY = 0; cellY < yCells; ++cellY)
				{
					for (int cellX = 0; cellX < xCells; ++cellX)
					{
						const BlockPos blockPos{
							cellX * lodScale,
							cellY * lodScale,
							cellZ * lodScale
						};
						representatives[representativeIndex(blockPos, lodScale)] =
							getRepresentativeBlockType(blockPos, lodScale);
					}
				}
			}
		}

		buildGreedyOpaqueMeshForLod(lodScale, representatives, chunkManager, vertices, indices);
		{
			ScopedProfileStage timer(ProfileStage::WaterMeshing);
			buildTransparentMeshForLod(lodScale, representatives, chunkManager, transparentVertices, transparentIndices);
		}
	}

	void Chunk::buildGreedyOpaqueMeshForLod(int lodScale, const std::vector<BlockTypeId>& representatives, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices)
	{
		buildGreedyMeshForLod(lodScale, false, representatives, chunkManager, vertices, indices);
	}

	void Chunk::buildGreedyMeshForLod(int lodScale, bool waterOnly, const std::vector<BlockTypeId>& representatives, ChunkManager& chunkManager, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices)
	{
		uint64_t maskNanoseconds = 0;
		uint64_t mergeNanoseconds = 0;
		for (int dirIndex = 0; dirIndex < 6; dirIndex++)
		{
			g::Direction dir = (g::Direction)dirIndex;
			int uCells = (dir == g::Direction::FRONT || dir == g::Direction::BACK || dir == g::Direction::TOP || dir == g::Direction::BOTTOM)
				? CHUNK_SIZE / lodScale
				: CHUNK_SIZE / lodScale;
			int vCells = (dir == g::Direction::TOP || dir == g::Direction::BOTTOM)
				? CHUNK_SIZE / lodScale
				: CHUNK_HEIGHT / lodScale;
			int sliceCells = (dir == g::Direction::FRONT || dir == g::Direction::BACK)
				? CHUNK_SIZE / lodScale
				: (dir == g::Direction::RIGHT || dir == g::Direction::LEFT)
					? CHUNK_SIZE / lodScale
					: CHUNK_HEIGHT / lodScale;

			std::vector<BlockTypeId> mask(uCells * vCells, BlockTypeId::AIR);

			for (int slice = 0; slice < sliceCells; slice++)
			{
				const auto maskStart = std::chrono::steady_clock::now();
				std::fill(mask.begin(), mask.end(), BlockTypeId::AIR);

				for (int v = 0; v < vCells; v++)
				{
					for (int u = 0; u < uCells; u++)
					{
						BlockPos blockPos;
						switch (dir)
						{
						case g::Direction::FRONT:
						case g::Direction::BACK:
							blockPos = { u * lodScale, v * lodScale, slice * lodScale };
							break;
						case g::Direction::RIGHT:
						case g::Direction::LEFT:
							blockPos = { slice * lodScale, v * lodScale, u * lodScale };
							break;
						case g::Direction::TOP:
						case g::Direction::BOTTOM:
							blockPos = { u * lodScale, slice * lodScale, v * lodScale };
							break;
						default:
							blockPos = {};
							break;
						}

						BlockTypeId blockTypeId = representatives[representativeIndex(blockPos, lodScale)];
						if (waterOnly ? blockTypeId != BlockTypeId::WATER : !isSolid(blockTypeId))
						{
							continue;
						}

						bool visible = lodScale == 1
							? isFaceVisible(blockTypeId, Face{ blockPos, dir }, chunkManager)
							: isLodFaceVisible(blockTypeId, Face{ blockPos, dir }, lodScale, chunkManager);
						if (visible)
						{
							mask[u + v * uCells] = blockTypeId;
							if (!waterOnly)
							{
								WorldProfiler::instance().increment(ProfileCounter::CandidateFaces);
							}
						}
					}
				}
				const auto mergeStart = std::chrono::steady_clock::now();
				maskNanoseconds += static_cast<uint64_t>(
					std::chrono::duration_cast<std::chrono::nanoseconds>(
						mergeStart - maskStart).count());

				for (int v = 0; v < vCells; v++)
				{
					for (int u = 0; u < uCells;)
					{
						BlockTypeId blockTypeId = mask[u + v * uCells];
						if (blockTypeId == BlockTypeId::AIR)
						{
							u++;
							continue;
						}

						int width = 1;
						while (u + width < uCells && mask[u + width + v * uCells] == blockTypeId)
						{
							width++;
						}

						int height = 1;
						bool canGrow = true;
						while (v + height < vCells && canGrow)
						{
							for (int x = 0; x < width; x++)
							{
								if (mask[u + x + (v + height) * uCells] != blockTypeId)
								{
									canGrow = false;
									break;
								}
							}

							if (canGrow)
							{
								height++;
							}
						}

						BlockPos blockPos;
						switch (dir)
						{
						case g::Direction::FRONT:
						case g::Direction::BACK:
							blockPos = { u * lodScale, v * lodScale, slice * lodScale };
							break;
						case g::Direction::RIGHT:
						case g::Direction::LEFT:
							blockPos = { slice * lodScale, v * lodScale, u * lodScale };
							break;
						case g::Direction::TOP:
						case g::Direction::BOTTOM:
							blockPos = { u * lodScale, slice * lodScale, v * lodScale };
							break;
						default:
							blockPos = {};
							break;
						}

						addGreedyFaceToMesh(blockTypeId, dir, blockPos, width, height, lodScale, vertices, indices);
						if (!waterOnly)
						{
							WorldProfiler::instance().increment(ProfileCounter::EmittedGreedyQuads);
						}

						for (int y = 0; y < height; y++)
						{
							for (int x = 0; x < width; x++)
							{
								mask[u + x + (v + y) * uCells] = BlockTypeId::AIR;
							}
						}

						u += width;
					}
				}
				mergeNanoseconds += static_cast<uint64_t>(
					std::chrono::duration_cast<std::chrono::nanoseconds>(
						std::chrono::steady_clock::now() - mergeStart).count());
			}
		}
		if (!waterOnly)
		{
			WorldProfiler::instance().record(ProfileStage::OpaqueMaskBuild, maskNanoseconds);
			WorldProfiler::instance().record(ProfileStage::GreedyMergeAndEmit, mergeNanoseconds);
		}
	}

	void Chunk::buildTransparentMeshForLod(int lodScale, const std::vector<BlockTypeId>& representatives, ChunkManager& chunkManager, std::vector<g::Vertex>& transparentVertices, std::vector<GLuint>& transparentIndices)
	{
		if (lodScale == 1)
		{
			buildGreedyMeshForLod(lodScale, true, representatives, chunkManager, transparentVertices, transparentIndices);
			return;
		}

		// Coarse water cells use their highest contained water block and remain
		// one block tall. Merging cells with different heights would create
		// sloped or floating sheets, so coarse LODs retain the sampled path.
		for (int z = 0; z < CHUNK_SIZE; z += lodScale)
		{
			for (int y = 0; y < CHUNK_HEIGHT; y += lodScale)
			{
				for (int x = 0; x < CHUNK_SIZE; x += lodScale)
				{
					BlockPos blockPos = { x, y, z };

					glm::vec3 blockPosVec = toVec3(blockPos);
					BlockTypeId blockTypeId = representatives[representativeIndex(blockPos, lodScale)];

					if (blockTypeId != BlockTypeId::WATER)
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
							addFaceToMesh(blockTypeId, dir, renderBlockPos + m_origin, blockPosVec, transparentVertices, transparentIndices, meshScale, lodScale);
						}
					}
				}
			}
		}
	}

	void Chunk::addGreedyFaceToMesh(BlockTypeId blockTypeId, g::Direction direction, BlockPos blockPos, int width, int height, int lodScale, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices)
	{
		glm::vec3 scale(lodScale);
		switch (direction)
		{
		case g::Direction::FRONT:
		case g::Direction::BACK:
			scale.x = width * lodScale;
			scale.y = height * lodScale;
			break;
		case g::Direction::RIGHT:
		case g::Direction::LEFT:
			scale.z = width * lodScale;
			scale.y = height * lodScale;
			break;
		case g::Direction::TOP:
		case g::Direction::BOTTOM:
			scale.x = width * lodScale;
			scale.z = height * lodScale;
			break;
		default:
			break;
		}

		addFaceToMesh(blockTypeId, direction, blockPos + m_origin, toVec3(blockPos), vertices, indices, scale, lodScale);
	}

	void Chunk::addFaceToMesh(BlockTypeId blockTypeId, g::Direction direction, BlockPos worldBlockPos, glm::vec3 blockPos, std::vector<g::Vertex>& vertices, std::vector<GLuint>& indices, glm::vec3 scale, int lodScale)
	{
		for (int j = 0; j < g::Quad::indexCount; j++)
		{
			indices.push_back(g::Quad::indices[j] + vertices.size());
		}

		int vertexPositionIndex = g::Quad::vertexPositionIndexMap.at(direction);
		int uvIndex = g::Quad::uvIndexMap.at(direction);
		glm::vec2 atlasTileCoords = g::getTextureAtlasTileCoords(getTextureForFace(blockTypeId, direction, lodScale));
		glm::vec2 uvRepeat(1.f);
		switch (direction)
		{
		case g::Direction::FRONT:
		case g::Direction::BACK:
			uvRepeat = glm::vec2(scale.x, scale.y);
			break;
		case g::Direction::RIGHT:
		case g::Direction::LEFT:
			uvRepeat = glm::vec2(scale.z, scale.y);
			break;
		case g::Direction::TOP:
		case g::Direction::BOTTOM:
			uvRepeat = glm::vec2(scale.x, scale.z);
			break;
		default:
			break;
		}

		for (int j = 0; j < g::Quad::vertexCount; j++)
		{
			glm::vec3 vertexPos = glm::vec3(
				g::Quad::vertexPositions[vertexPositionIndex + j * 3],
				g::Quad::vertexPositions[vertexPositionIndex + j * 3 + 1],
				g::Quad::vertexPositions[vertexPositionIndex + j * 3 + 2]
			);
			glm::vec2 uv = glm::vec2(
				g::Quad::uvs[uvIndex + j * 2] * uvRepeat.x,
				g::Quad::uvs[uvIndex + j * 2 + 1] * uvRepeat.y
			);

			vertices.push_back(g::Vertex(
				blockPos + vertexPos * scale,
				g::getNormal(direction),
				uv,
				blockTypeId,
				worldBlockPos,
				true,
				atlasTileCoords
			));
		}
	}

	void Chunk::putBlock(Block block)
	{
		const int blockIndex = to1dIndex(block.pos);
		const size_t bitIndex = static_cast<size_t>(blockIndex) * BITS_PER_BLOCK;
		const size_t byteIndex = bitIndex / 8;
		const int bitOffset = static_cast<int>(bitIndex % 8);
		uint16_t packed = static_cast<uint16_t>(m_blocks[byteIndex])
			| (static_cast<uint16_t>(m_blocks[byteIndex + 1]) << 8);
		const uint16_t shiftedMask = static_cast<uint16_t>(BLOCK_ID_MASK << bitOffset);
		packed = static_cast<uint16_t>((packed & ~shiftedMask)
			| ((static_cast<uint16_t>(block.type) & BLOCK_ID_MASK) << bitOffset));
		m_blocks[byteIndex] = static_cast<uint8_t>(packed & 0xff);
		m_blocks[byteIndex + 1] = static_cast<uint8_t>(packed >> 8);
	}

	BlockTypeId Chunk::getBlock(int x, int y, int z)
	{
		const int blockIndex = to1dIndex(x, y, z);
		const size_t bitIndex = static_cast<size_t>(blockIndex) * BITS_PER_BLOCK;
		const size_t byteIndex = bitIndex / 8;
		const int bitOffset = static_cast<int>(bitIndex % 8);
		const uint16_t packed = static_cast<uint16_t>(m_blocks[byteIndex])
			| (static_cast<uint16_t>(m_blocks[byteIndex + 1]) << 8);
		const uint8_t value = static_cast<uint8_t>((packed >> bitOffset) & BLOCK_ID_MASK);
		return static_cast<BlockTypeId>(value);
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
		if (m_meshes[lodIndex] != nullptr || m_pendingMeshReady[lodIndex] || m_lodBuildQueued[lodIndex])
		{
			return false;
		}

		m_lodBuildQueued[lodIndex] = true;
		return true;
	}

	bool Chunk::hasPendingMeshUpload()
	{
		std::unique_lock<std::mutex> lock = acquireLock();
		for (bool pending : m_pendingMeshReady)
		{
			if (pending)
			{
				return true;
			}
		}

		return false;
	}

	int Chunk::uploadPendingMeshes()
	{
		for (int lodIndex = 0; lodIndex < CHUNK_LOD_LEVEL_COUNT; lodIndex++)
		{
			std::shared_ptr<g::Mesh> mesh;
			std::shared_ptr<g::Mesh> transparentMesh;

			{
				std::unique_lock<std::mutex> lock = acquireLock();
				if (!m_pendingMeshReady[lodIndex])
				{
					continue;
				}

				mesh = m_pendingMeshes[lodIndex];
				transparentMesh = m_pendingTransparentMeshes[lodIndex];
			}

			if (mesh)
			{
				mesh->upload();
			}
			if (transparentMesh)
			{
				transparentMesh->upload();
			}

			{
				std::unique_lock<std::mutex> lock = acquireLock();
				m_meshes.fill(nullptr);
				m_transparentMeshes.fill(nullptr);
				m_meshes[lodIndex] = std::move(mesh);
				m_transparentMeshes[lodIndex] = std::move(transparentMesh);
				m_pendingMeshes[lodIndex] = nullptr;
				m_pendingTransparentMeshes[lodIndex] = nullptr;
				m_pendingMeshReady[lodIndex] = false;
			}

			return 1;
		}

		return 0;
	}

	std::shared_ptr<g::Mesh> Chunk::getMesh(ChunkLod lod)
	{
		std::shared_ptr<g::Mesh> mesh = m_meshes[toLodIndex(lod)];
		if (mesh)
		{
			return mesh;
		}
		for (const std::shared_ptr<g::Mesh>& fallback : m_meshes)
		{
			if (fallback)
			{
				return fallback;
			}
		}
		return nullptr;
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh()
	{
		return getTransparentMesh(ChunkLod::FULL);
	}

	std::shared_ptr<g::Mesh> Chunk::getTransparentMesh(ChunkLod lod)
	{
		std::shared_ptr<g::Mesh> mesh = m_transparentMeshes[toLodIndex(lod)];
		if (mesh)
		{
			return mesh;
		}
		for (const std::shared_ptr<g::Mesh>& fallback : m_transparentMeshes)
		{
			if (fallback)
			{
				return fallback;
			}
		}
		return nullptr;
	}

	bool Chunk::hasAnyMesh()
	{
		for (const std::shared_ptr<g::Mesh>& mesh : m_meshes)
		{
			if (mesh)
			{
				return true;
			}
		}
		return false;
	}

	void Chunk::accumulateMemoryDiagnostics(MemoryDiagnostics& diagnostics)
	{
		std::unique_lock<std::mutex> lock = acquireLock();
		diagnostics.voxelStorageBytes += m_blocks.size() * sizeof(uint8_t);

		auto addResidentMesh = [&](const std::shared_ptr<g::Mesh>& mesh)
		{
			if (!mesh)
			{
				return;
			}
			diagnostics.residentMeshCount++;
			diagnostics.meshVertexCount += mesh->getVertexCount();
			diagnostics.meshIndexCount += mesh->getIndexCount();
			diagnostics.estimatedGpuMeshBytes += mesh->getEstimatedGpuBytes();
		};
		auto addPendingMesh = [&](const std::shared_ptr<g::Mesh>& mesh)
		{
			if (!mesh)
			{
				return;
			}
			diagnostics.pendingMeshCount++;
			diagnostics.pendingCpuMeshBytes += mesh->getCpuStorageBytes();
		};

		for (const std::shared_ptr<g::Mesh>& mesh : m_meshes)
		{
			addResidentMesh(mesh);
		}
		for (const std::shared_ptr<g::Mesh>& mesh : m_transparentMeshes)
		{
			addResidentMesh(mesh);
		}
		for (const std::shared_ptr<g::Mesh>& mesh : m_pendingMeshes)
		{
			addPendingMesh(mesh);
		}
		for (const std::shared_ptr<g::Mesh>& mesh : m_pendingTransparentMeshes)
		{
			addPendingMesh(mesh);
		}
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
		WorldProfiler::instance().increment(ProfileCounter::FaceVisibilityChecks);
		BlockPos neighbourPos = face.pos + toBlockPos(getNormal(face.dir));
		BlockTypeId neighbour;

		if (neighbourPos.y < 0 || neighbourPos.y >= CHUNK_HEIGHT)
		{
			return true;
		}
		if (isBlockInBounds(neighbourPos))
		{
			WorldProfiler::instance().increment(ProfileCounter::LocalMeshingBlockReads);
			neighbour = getBlock(neighbourPos);
		}
		else
		{
			Chunk* neighbourChunk = getNeighbourChunk(face.dir, chunkManager);

			if (neighbourChunk == nullptr)
			{
				WorldProfiler::instance().increment(ProfileCounter::WorldFallbackReads);
				neighbour = m_world->getBlock(m_origin + neighbourPos);
			}
			else 
			{
				WorldProfiler::instance().increment(ProfileCounter::NeighbourChunkReads);
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
		WorldProfiler::instance().increment(ProfileCounter::FaceVisibilityChecks);
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
		WorldProfiler::instance().increment(ProfileCounter::RepresentativeBlockSamples);
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
					WorldProfiler::instance().increment(ProfileCounter::LocalMeshingBlockReads);
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
					WorldProfiler::instance().increment(ProfileCounter::LocalMeshingBlockReads);
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
			WorldProfiler::instance().increment(ProfileCounter::LocalMeshingBlockReads);
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
			WorldProfiler::instance().increment(ProfileCounter::NeighbourChunkReads);
			return neighbourChunk->getBlock(worldPosToLocalPos(worldPos));
		}

		WorldProfiler::instance().increment(ProfileCounter::WorldFallbackReads);
		return m_world->getBlock(worldPos);
	}
}
