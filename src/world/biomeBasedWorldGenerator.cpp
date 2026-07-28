#include "biomeBasedWorldGenerator.hpp"
#include "worldProfiler.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr float RIVER_SURFACE_CLEARANCE = 1.0f;
		constexpr float HEADWATER_BED_DEPTH = 2.0f;
		constexpr float MAJOR_RIVER_BED_DEPTH = 5.0f;
		constexpr float RIVER_DENSITY_SCALE = 0.45f;
		constexpr float RIVER_BANK_MASK_THRESHOLD = 0.35f;
		constexpr float RIVER_WATER_MASK_THRESHOLD = 0.45f;
		constexpr float RIVER_SAND_MAX_SLOPE = 2.5f;

		constexpr float COAST_MASK_THRESHOLD = 0.35f;
		constexpr int BEACH_MAX_HEIGHT_ABOVE_WATER = 2;
		constexpr float BEACH_SAND_MAX_SLOPE = 2.0f;
		constexpr float STONE_SURFACE_MIN_SLOPE = 4.0f;
		constexpr float GRAVEL_SURFACE_MIN_SLOPE = 2.5f;
		constexpr float CENTRAL_DIFFERENCE_SCALE = 0.5f;
		constexpr int BEDROCK_THICKNESS = 4;
		constexpr int COLUMN_SAMPLE_BORDER = 1;

		bool isFrozenBiome(BiomeType biome)
		{
			return biome == BiomeType::SnowyTundra || biome == BiomeType::Glacier;
		}

		BlockTypeId getWaterBlockType(
			const Biome* biome, const TerrainSample& terrain, int y)
		{
			const int surfaceY = terrain.riverMask > RIVER_WATER_MASK_THRESHOLD
				? static_cast<int>(std::floor(terrain.riverSurfaceHeight))
				: WATER_HEIGHT;
			return isFrozenBiome(biome->type) && y == surfaceY
				? BlockTypeId::ICE
				: BlockTypeId::WATER;
		}

		float calculateSlopeFromHeights(int west, int east, int north, int south)
		{
			return CENTRAL_DIFFERENCE_SCALE
				* std::max(std::abs(east - west), std::abs(south - north));
		}
	}

	BiomeBasedWorldGenerator::BiomeBasedWorldGenerator(long seed)
		: m_biomeSelector(seed),
		  m_terrainGenerator(seed, [this](const TerrainSample& terrain) {
			  return m_biomeSelector.getWeight(terrain, BiomeType::Plains);
		  }),
		  m_hydrologyGenerator(seed, [this](int x, int z) {
			  const TerrainSample terrain = m_terrainGenerator.sampleBase(x, z);
			  return HydrologyTerrainSample{
				  terrain.height,
				  terrain.valleyMask,
				  terrain.canyonMask
			  };
		  }),
		  m_caveGenerator(seed) {}

	float BiomeBasedWorldGenerator::calcContinentalness(int x, int z)
	{
		return m_terrainGenerator.sampleContinentalness(x, z);
	}

	float BiomeBasedWorldGenerator::calcErosion(int x, int z)
	{
		return m_terrainGenerator.sampleErosion(x, z);
	}

	float BiomeBasedWorldGenerator::calcTemperature(int x, int z)
	{
		return m_terrainGenerator.sampleTemperature(x, z);
	}

	float BiomeBasedWorldGenerator::calcHumidity(int x, int z)
	{
		return m_terrainGenerator.sampleHumidity(x, z);
	}

	std::vector<BiomeWeight> BiomeBasedWorldGenerator::buildWeights(int x, int z)
	{
		return buildWeights(sampleTerrain(x, z));
	}

	std::vector<BiomeWeight> BiomeBasedWorldGenerator::buildWeights(const TerrainSample& terrain)
	{
		return m_biomeSelector.buildWeights(terrain);
	}

	std::vector<BiomeWeight> BiomeBasedWorldGenerator::normalizeWeights(std::vector<BiomeWeight> weights)
	{
		return m_biomeSelector.normalize(std::move(weights));
	}

	int BiomeBasedWorldGenerator::calculateHeight(int x, int z)
	{
		auto terrain = sampleTerrain(x, z);
		return calculateSurfaceHeight(terrain, x, z);
	}

	int BiomeBasedWorldGenerator::calculateSurfaceHeight(const TerrainSample& terrain, int x, int z)
	{
		return calculateSurfaceHeight(terrain, m_caveGenerator.sampleColumn(x, z), x, z);
	}

	int BiomeBasedWorldGenerator::calculateSurfaceHeight(const TerrainSample& terrain, const CaveColumnSample& caveColumn, int x, int z)
	{
		ScopedProfileStage timer(ProfileStage::SurfaceHeightSearch);
		float maximumDisplacement = terrain.densityStrength + terrain.formationStrength;
		int searchTop = std::min(WORLD_HEIGHT - 1, static_cast<int>(std::ceil(terrain.height + maximumDisplacement)));
		for (int y = searchTop; y >= 0; --y)
		{
			if (calculateDensity(terrain, caveColumn, x, y, z) > 0.0f)
			{
				return y;
			}
		}
		return 0;
	}

	TerrainSample BiomeBasedWorldGenerator::sampleTerrain(int x, int z)
	{
		TerrainSample result = m_terrainGenerator.sampleBase(x, z);
		HydrologySample hydrology = m_hydrologyGenerator.sample(x, z);
		result.riverMask = hydrology.mask;
		result.riverFlow = hydrology.flow;
		result.riverSurfaceHeight = std::min(
			hydrology.waterSurfaceHeight, result.height - RIVER_SURFACE_CLEARANCE);

		if (result.riverMask > 0.0f)
		{
			constexpr float RIVER_EROSION_OVERLAP_REDUCTION = 0.80f;
			const float erosionCarving =
				std::min(0.0f, result.erosionHeightDelta);
			result.height -= erosionCarving
				* result.riverMask
				* RIVER_EROSION_OVERLAP_REDUCTION;
			float riverBedDepth = utils::lerp(
				HEADWATER_BED_DEPTH, MAJOR_RIVER_BED_DEPTH, result.riverFlow);
			float riverBedHeight = result.riverSurfaceHeight - riverBedDepth;
			result.height = utils::lerp(result.height, std::min(result.height, riverBedHeight), result.riverMask);
			result.densityStrength *= utils::lerp(1.0f, RIVER_DENSITY_SCALE, result.riverMask);
			result.formationStrength *= 1.0f - result.riverMask;
			result.formationMask *= 1.0f - result.riverMask;
		}

		return result;
	}

	float BiomeBasedWorldGenerator::calculateSlope(int x, int z)
	{
		int west = calculateSurfaceHeight(sampleTerrain(x - 1, z), x - 1, z);
		int east = calculateSurfaceHeight(sampleTerrain(x + 1, z), x + 1, z);
		int north = calculateSurfaceHeight(sampleTerrain(x, z - 1), x, z - 1);
		int south = calculateSurfaceHeight(sampleTerrain(x, z + 1), x, z + 1);
		return calculateSlopeFromHeights(west, east, north, south);
	}

	BlockTypeId BiomeBasedWorldGenerator::getSurfaceBlockType(const Biome* biome, const TerrainSample& terrain, float slope, int x, int z, int surfaceHeight)
	{
		if (terrain.riverMask > RIVER_BANK_MASK_THRESHOLD)
		{
			return slope < RIVER_SAND_MAX_SLOPE ? BlockTypeId::SAND : BlockTypeId::GRAVEL;
		}
		BlockTypeId biomeSurfaceBlock = biome->blockFunc(x, surfaceHeight, z, surfaceHeight);
		return selectSurfaceBlock(biomeSurfaceBlock, terrain.coastMask, slope, surfaceHeight);
	}

	BlockTypeId BiomeBasedWorldGenerator::selectSurfaceBlock(BlockTypeId biomeSurfaceBlock, float coastMask, float slope, int surfaceHeight)
	{
		if (surfaceHeight < WATER_HEIGHT) return biomeSurfaceBlock;
		switch (biomeSurfaceBlock)
		{
		case BlockTypeId::SNOW:
		case BlockTypeId::PODZOL:
		case BlockTypeId::DRY_GRASS:
		case BlockTypeId::RAINFOREST_GRASS:
		case BlockTypeId::BASALT:
		case BlockTypeId::LAVA:
		case BlockTypeId::SALT:
		case BlockTypeId::TERRACOTTA:
			return biomeSurfaceBlock;
		default:
			break;
		}
		if (coastMask > COAST_MASK_THRESHOLD
			&& surfaceHeight <= WATER_HEIGHT + BEACH_MAX_HEIGHT_ABOVE_WATER)
		{
			return slope < BEACH_SAND_MAX_SLOPE ? BlockTypeId::SAND : BlockTypeId::GRAVEL;
		}
		if (slope >= STONE_SURFACE_MIN_SLOPE)
		{
			return BlockTypeId::STONE;
		}
		if (slope >= GRAVEL_SURFACE_MIN_SLOPE)
		{
			return BlockTypeId::GRAVEL;
		}
		return biomeSurfaceBlock;
	}

	float BiomeBasedWorldGenerator::calculateDensity(const TerrainSample& terrain, int x, int y, int z)
	{
		return calculateDensity(terrain, m_caveGenerator.sampleColumn(x, z), x, y, z);
	}

	float BiomeBasedWorldGenerator::calculateDensity(const TerrainSample& terrain, const CaveColumnSample& caveColumn, int x, int y, int z)
	{
		WorldProfiler::instance().increment(ProfileCounter::TerrainDensityEvaluations);
		float terrainDensity = m_terrainGenerator.sampleDensity(terrain, x, y, z);
		if (terrainDensity <= 0.0f)
		{
			WorldProfiler::instance().increment(ProfileCounter::TerrainAirCaveSkips);
			return terrainDensity;
		}
		WorldProfiler::instance().increment(ProfileCounter::CaveEvaluations);
		float caveDensity = m_caveGenerator.sampleDensity(caveColumn, x, y, z, terrain.height);
		return std::min(terrainDensity, caveDensity);
	}

	bool BiomeBasedWorldGenerator::isWater(const TerrainSample& terrain, int y) const
	{
		return (y <= WATER_HEIGHT && terrain.height < WATER_HEIGHT)
			|| (terrain.riverMask > RIVER_WATER_MASK_THRESHOLD
				&& y <= static_cast<int>(std::floor(terrain.riverSurfaceHeight)));
	}

	TerrainDebugSample BiomeBasedWorldGenerator::getTerrainDebugSample(int x, int z)
	{
		auto terrain = sampleTerrain(x, z);
		auto dominant = std::max_element(terrain.terrainWeights.begin(), terrain.terrainWeights.end());
		int surfaceHeight = calculateSurfaceHeight(terrain, x, z);
		TerrainType dominantTerrain =
			static_cast<TerrainType>(std::distance(terrain.terrainWeights.begin(), dominant));
		if (terrain.continentalness < 0.48f && surfaceHeight <= WATER_HEIGHT)
		{
			dominantTerrain = TerrainType::Ocean;
		}
		else if (terrain.canyonMask >= 0.30f)
		{
			dominantTerrain = TerrainType::Canyons;
		}
		else if (terrain.valleyMask >= 0.30f)
		{
			dominantTerrain = TerrainType::Valleys;
		}
		else if (terrain.mountainCoreMask >= 0.20f
			|| terrain.mountainRangeMask >= 0.45f)
		{
			dominantTerrain = TerrainType::Mountains;
		}
		else if (terrain.mountainFoothillMask >= 0.25f)
		{
			dominantTerrain = TerrainType::Highlands;
		}
		float slope = 0.0f;
		BlockTypeId surfaceBlock = BlockTypeId::WATER;
		auto weights = buildWeights(terrain);
		const Biome* selectedBiome = selectBiome(weights, terrain, x, z);
		if (surfaceHeight > WATER_HEIGHT)
		{
			surfaceBlock = selectedBiome->blockFunc(x, surfaceHeight, z, surfaceHeight);
		}
		return {
			terrain.continentalness,
			terrain.erosion,
			terrain.temperature,
			terrain.humidity,
			terrain.ridge,
			terrain.mountainRangeMask,
			terrain.mountainFoothillMask,
			terrain.mountainCoreMask,
			terrain.mountainPeakMask,
			terrain.mountainPassMask,
			terrain.valleyMask,
			terrain.canyonMask,
			terrain.erosionGullyMask,
			terrain.talusMask,
			terrain.depositionMask,
			terrain.coastMask,
			slope,
			terrain.plateauCliffMask,
			terrain.formationMask,
			terrain.riverMask,
			terrain.riverFlow,
			terrain.height,
			terrain.densityStrength,
			terrain.terrainWeights,
			dominantTerrain,
			selectedBiome->type,
			surfaceHeight,
			surfaceBlock
		};
	}

	const Biome* BiomeBasedWorldGenerator::selectBiome(
		std::vector<BiomeWeight> weights,
		const TerrainSample& terrain,
		int x,
		int z)
	{
		return m_biomeSelector.select(std::move(weights), terrain, x, z);
	}

	int BiomeBasedWorldGenerator::getHeight(int x, int z)
	{
		return m_heightMap.withLock([&](auto &map) -> int {
			auto &row = map[x];
			if (auto it = row.find(z); it != row.end())
			{
				return it->second;
			}
        	return row[z] = calculateHeight(x, z);
		});
	}

	BlockTypeId BiomeBasedWorldGenerator::getBlockType(BlockPos pos)
	{
		int x = pos.x, z = pos.z, y = pos.y;
		if (y < BEDROCK_THICKNESS) return BlockTypeId::BEDROCK;

		auto terrain = sampleTerrain(x, z);
		int surfaceHeight = getHeight(x, z);
		auto weights = buildWeights(terrain);
		const Biome* dominantBiome = selectBiome(weights, terrain, x, z);

		if (calculateDensity(terrain, x, y, z) <= 0.0f)
		{
			return isWater(terrain, y)
				? getWaterBlockType(dominantBiome, terrain, y)
				: BlockTypeId::AIR;
		}

		if (y == surfaceHeight)
		{
			return getSurfaceBlockType(dominantBiome, terrain, calculateSlope(x, z), x, z, surfaceHeight);
		}
		return dominantBiome->blockFunc(x, y, z, surfaceHeight);
	}

	void BiomeBasedWorldGenerator::generateChunkData(Chunk &chunk)
	{
		ScopedProfileCounterBatch counterBatch;
		auto origin = chunk.getOrigin();
		constexpr int PADDED_SIZE = CHUNK_SIZE + COLUMN_SAMPLE_BORDER * 2;
		struct ColumnData
		{
			TerrainSample terrain;
			CaveColumnSample cave;
			int surfaceHeight;
		};
		std::array<ColumnData, PADDED_SIZE * PADDED_SIZE> columns;
		auto columnAt = [&](int localX, int localZ) -> const ColumnData&
		{
			int paddedX = localX + COLUMN_SAMPLE_BORDER;
			int paddedZ = localZ + COLUMN_SAMPLE_BORDER;
			return columns[paddedX + paddedZ * PADDED_SIZE];
		};

		{
			ScopedProfileStage timer(ProfileStage::ColumnSampling);
			for (int paddedX = 0; paddedX < PADDED_SIZE; ++paddedX)
			{
				for (int paddedZ = 0; paddedZ < PADDED_SIZE; ++paddedZ)
				{
					int worldX = origin.x + paddedX - COLUMN_SAMPLE_BORDER;
					int worldZ = origin.z + paddedZ - COLUMN_SAMPLE_BORDER;
					auto terrain = sampleTerrain(worldX, worldZ);
					CaveColumnSample cave = m_caveGenerator.sampleColumn(worldX, worldZ);
					columns[paddedX + paddedZ * PADDED_SIZE] = {
						terrain,
						cave,
						calculateSurfaceHeight(terrain, cave, worldX, worldZ)
					};
				}
			}
		}

		{
			ScopedProfileStage timer(ProfileStage::VoxelGeneration);
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				for (int z = 0; z < CHUNK_SIZE; z++)
				{
					int worldX = origin.x + x;
					int worldZ = origin.z + z;
					const auto& column = columnAt(x, z);
					const auto& terrain = column.terrain;
					auto weights = buildWeights(terrain);
					const Biome* dominantBiome =
						selectBiome(weights, terrain, worldX, worldZ);
					int surfaceHeight = column.surfaceHeight;
					int west = columnAt(x - 1, z).surfaceHeight;
					int east = columnAt(x + 1, z).surfaceHeight;
					int north = columnAt(x, z - 1).surfaceHeight;
					int south = columnAt(x, z + 1).surfaceHeight;
					float slope = calculateSlopeFromHeights(west, east, north, south);
					BlockTypeId surfaceBlockType = getSurfaceBlockType(dominantBiome, terrain, slope, worldX, worldZ, surfaceHeight);

					for (int y = 0; y < CHUNK_HEIGHT; y++)
					{
						int worldY = origin.y + y;
						BlockTypeId blockTypeId;
						if (worldY < BEDROCK_THICKNESS)
						{
							blockTypeId = BlockTypeId::BEDROCK;
						}
						else if (calculateDensity(terrain, column.cave, worldX, worldY, worldZ) <= 0.0f)
						{
							blockTypeId = isWater(terrain, worldY)
								? getWaterBlockType(dominantBiome, terrain, worldY)
								: BlockTypeId::AIR;
						}
						else
						{
							blockTypeId = worldY == surfaceHeight
								? surfaceBlockType
								: dominantBiome->blockFunc(worldX, worldY, worldZ, surfaceHeight);
						}

						chunk.putBlock(Block{ BlockPos{ x, y, z }, blockTypeId });
					}
				}
			}
			WorldProfiler::instance().increment(ProfileCounter::BlockAssignments, CHUNK_BLOCK_COUNT);
		}
	}

}
