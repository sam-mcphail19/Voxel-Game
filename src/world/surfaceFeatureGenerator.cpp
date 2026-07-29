#include "surfaceFeatureGenerator.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>

#include "chunk.hpp"
#include "../constants.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr int FEATURE_CELL_SIZE = 12;
		constexpr int MAX_FEATURE_RADIUS = 3;
		constexpr float MAX_TREE_SLOPE = 1.5f;
		constexpr float MAX_CACTUS_SLOPE = 1.0f;
		constexpr float MAX_BOULDER_SLOPE = 3.0f;

		constexpr uint32_t FEATURE_CHANCE_DENOMINATOR = 1000;
		constexpr uint32_t PLAINS_TREE_CHANCE = 115;
		constexpr uint32_t TAIGA_TREE_CHANCE = 360;
		constexpr uint32_t RAINFOREST_TREE_CHANCE = 520;
		constexpr uint32_t SAVANNA_TREE_CHANCE = 80;
		constexpr uint32_t DESERT_CACTUS_CHANCE = 190;
		constexpr uint32_t DEFAULT_BOULDER_CHANCE = 55;
		constexpr uint32_t ROCKY_BOULDER_CHANCE = 260;
		constexpr uint32_t VOLCANIC_OUTCROP_CHANCE = 220;
		constexpr uint32_t GLACIER_ICE_CHANCE = 160;

		int floorDiv(int value, int divisor)
		{
			int quotient = value / divisor;
			int remainder = value % divisor;
			return remainder < 0 ? quotient - 1 : quotient;
		}

		uint64_t mix64(uint64_t value)
		{
			value ^= value >> 30;
			value *= 0xbf58476d1ce4e5b9ULL;
			value ^= value >> 27;
			value *= 0x94d049bb133111ebULL;
			return value ^ (value >> 31);
		}

		uint64_t featureHash(long seed, int cellX, int cellZ)
		{
			uint64_t value = static_cast<uint64_t>(seed);
			value ^= mix64(static_cast<uint64_t>(static_cast<int64_t>(cellX)));
			value ^= mix64(static_cast<uint64_t>(static_cast<int64_t>(cellZ)) + 0x9e3779b97f4a7c15ULL);
			return mix64(value);
		}

		bool isReplaceable(BlockTypeId block)
		{
			return block == BlockTypeId::AIR;
		}

		void putWorldBlock(Chunk& chunk, int worldX, int worldY, int worldZ, BlockTypeId type)
		{
			const BlockPos origin = chunk.getOrigin();
			const BlockPos local{
				worldX - origin.x,
				worldY - origin.y,
				worldZ - origin.z
			};
			if (local.x < 0 || local.x >= CHUNK_SIZE
				|| local.y < 0 || local.y >= CHUNK_HEIGHT
				|| local.z < 0 || local.z >= CHUNK_SIZE)
			{
				return;
			}
			if (isReplaceable(chunk.getBlock(local)))
			{
				chunk.putBlock(Block{local, type});
			}
		}

		void placeDeciduousTree(
			Chunk& chunk, int x, int groundY, int z, uint64_t hash)
		{
			const int trunkHeight = 4 + static_cast<int>((hash >> 8) % 3);
			for (int y = 1; y <= trunkHeight; ++y)
			{
				putWorldBlock(
					chunk, x, groundY + y, z, BlockTypeId::DECIDUOUS_LOG);
			}

			const int crownY = groundY + trunkHeight;
			for (int dy = -2; dy <= 1; ++dy)
			{
				const int radius = dy == 1 ? 1 : 2;
				for (int dx = -radius; dx <= radius; ++dx)
				{
					for (int dz = -radius; dz <= radius; ++dz)
					{
						if (radius == 2 && std::abs(dx) == 2 && std::abs(dz) == 2)
						{
							continue;
						}
						if (dx == 0 && dz == 0 && dy <= 0) continue;
						putWorldBlock(
							chunk,
							x + dx,
							crownY + dy,
							z + dz,
							BlockTypeId::DECIDUOUS_LEAVES);
					}
				}
			}
		}

		void placeConifer(Chunk& chunk, int x, int groundY, int z, uint64_t hash)
		{
			const int trunkHeight = 6 + static_cast<int>((hash >> 10) % 4);
			for (int y = 1; y <= trunkHeight; ++y)
			{
				putWorldBlock(chunk, x, groundY + y, z, BlockTypeId::CONIFER_LOG);
			}
			for (int layer = 0; layer < trunkHeight - 2; ++layer)
			{
				const int y = groundY + trunkHeight - layer;
				const int radius = std::min(2, 1 + layer / 2);
				for (int dx = -radius; dx <= radius; ++dx)
				{
					for (int dz = -radius; dz <= radius; ++dz)
					{
						if (std::abs(dx) + std::abs(dz) > radius + 1) continue;
						if (dx == 0 && dz == 0) continue;
						putWorldBlock(
							chunk, x + dx, y, z + dz, BlockTypeId::CONIFER_LEAVES);
					}
				}
			}
			putWorldBlock(
				chunk,
				x,
				groundY + trunkHeight + 1,
				z,
				BlockTypeId::CONIFER_LEAVES);
		}

		void placeCactus(Chunk& chunk, int x, int groundY, int z, uint64_t hash)
		{
			const int height = 2 + static_cast<int>((hash >> 12) % 3);
			for (int y = 1; y <= height; ++y)
			{
				putWorldBlock(chunk, x, groundY + y, z, BlockTypeId::CACTUS);
			}
		}

		void placeBoulder(
			Chunk& chunk,
			int x,
			int groundY,
			int z,
			uint64_t hash,
			BlockTypeId material)
		{
			const int radius = 1 + static_cast<int>((hash >> 14) % 2);
			for (int dx = -radius; dx <= radius; ++dx)
			{
				for (int dy = 0; dy <= radius; ++dy)
				{
					for (int dz = -radius; dz <= radius; ++dz)
					{
						const int distance = dx * dx + dz * dz + dy * dy * 2;
						if (distance > radius * radius + 1) continue;
						putWorldBlock(
							chunk, x + dx, groundY + 1 + dy, z + dz, material);
					}
				}
			}
		}

		void placeIceSpike(Chunk& chunk, int x, int groundY, int z, uint64_t hash)
		{
			const int height = 3 + static_cast<int>((hash >> 16) % 5);
			for (int y = 1; y <= height; ++y)
			{
				putWorldBlock(chunk, x, groundY + y, z, BlockTypeId::ICE);
			}
			if (height >= 6)
			{
				putWorldBlock(chunk, x + 1, groundY + 1, z, BlockTypeId::ICE);
				putWorldBlock(chunk, x, groundY + 1, z + 1, BlockTypeId::ICE);
			}
		}
	}

	SurfaceFeatureGenerator::SurfaceFeatureGenerator(long seed)
		: m_seed(seed) {}

	void SurfaceFeatureGenerator::generate(
		Chunk& chunk,
		const ColumnSampler& sampleColumn) const
	{
		const BlockPos origin = chunk.getOrigin();
		const int minCellX = floorDiv(origin.x - MAX_FEATURE_RADIUS, FEATURE_CELL_SIZE);
		const int maxCellX =
			floorDiv(origin.x + CHUNK_SIZE - 1 + MAX_FEATURE_RADIUS, FEATURE_CELL_SIZE);
		const int minCellZ = floorDiv(origin.z - MAX_FEATURE_RADIUS, FEATURE_CELL_SIZE);
		const int maxCellZ =
			floorDiv(origin.z + CHUNK_SIZE - 1 + MAX_FEATURE_RADIUS, FEATURE_CELL_SIZE);

		for (int cellX = minCellX; cellX <= maxCellX; ++cellX)
		{
			for (int cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ)
			{
				const uint64_t hash = featureHash(m_seed, cellX, cellZ);
				const int x = cellX * FEATURE_CELL_SIZE
					+ static_cast<int>((hash >> 20) % FEATURE_CELL_SIZE);
				const int z = cellZ * FEATURE_CELL_SIZE
					+ static_cast<int>((hash >> 28) % FEATURE_CELL_SIZE);
				const SurfaceFeatureColumn column = sampleColumn(x, z);
				if (column.underwater || column.surfaceHeight <= WATER_HEIGHT) continue;

				const uint32_t roll =
					static_cast<uint32_t>(hash % FEATURE_CHANCE_DENOMINATOR);
				switch (column.biome)
				{
				case BiomeType::Plains:
					if (column.slope <= MAX_TREE_SLOPE && roll < PLAINS_TREE_CHANCE)
					{
						placeDeciduousTree(chunk, x, column.surfaceHeight, z, hash);
					}
					else if (column.slope <= MAX_BOULDER_SLOPE
						&& roll < PLAINS_TREE_CHANCE + DEFAULT_BOULDER_CHANCE)
					{
						placeBoulder(
							chunk, x, column.surfaceHeight, z, hash, BlockTypeId::STONE);
					}
					break;
				case BiomeType::Taiga:
					if (column.slope <= MAX_TREE_SLOPE && roll < TAIGA_TREE_CHANCE)
					{
						placeConifer(chunk, x, column.surfaceHeight, z, hash);
					}
					break;
				case BiomeType::Rainforest:
					if (column.slope <= MAX_TREE_SLOPE && roll < RAINFOREST_TREE_CHANCE)
					{
						placeDeciduousTree(chunk, x, column.surfaceHeight, z, hash);
					}
					break;
				case BiomeType::Savanna:
					if (column.slope <= MAX_TREE_SLOPE && roll < SAVANNA_TREE_CHANCE)
					{
						placeDeciduousTree(chunk, x, column.surfaceHeight, z, hash);
					}
					break;
				case BiomeType::Desert:
				case BiomeType::Badlands:
					if (column.slope <= MAX_CACTUS_SLOPE && roll < DESERT_CACTUS_CHANCE)
					{
						placeCactus(chunk, x, column.surfaceHeight, z, hash);
					}
					break;
				case BiomeType::RockyHighlands:
				case BiomeType::Mountains:
					if (column.slope <= MAX_BOULDER_SLOPE && roll < ROCKY_BOULDER_CHANCE)
					{
						placeBoulder(
							chunk, x, column.surfaceHeight, z, hash, BlockTypeId::STONE);
					}
					break;
				case BiomeType::Volcanic:
					if (column.slope <= MAX_BOULDER_SLOPE
						&& roll < VOLCANIC_OUTCROP_CHANCE)
					{
						placeBoulder(
							chunk, x, column.surfaceHeight, z, hash, BlockTypeId::BASALT);
					}
					break;
				case BiomeType::Glacier:
				case BiomeType::SnowyTundra:
					if (column.slope <= MAX_BOULDER_SLOPE && roll < GLACIER_ICE_CHANCE)
					{
						placeIceSpike(chunk, x, column.surfaceHeight, z, hash);
					}
					break;
				default:
					if (column.slope <= MAX_BOULDER_SLOPE
						&& roll < DEFAULT_BOULDER_CHANCE)
					{
						placeBoulder(
							chunk, x, column.surfaceHeight, z, hash, BlockTypeId::GRAVEL);
					}
					break;
				}
			}
		}
	}
}
