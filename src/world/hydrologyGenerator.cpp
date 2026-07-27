#include "hydrologyGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <utility>
#include <vector>
#include <glm/common.hpp>
#include "../constants.hpp"
#include "../util/mathUtils.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr int REGION_SIZE = 256;
		constexpr int CELL_SIZE = 8;
		constexpr int CORE_CELLS = REGION_SIZE / CELL_SIZE;
		constexpr int PADDING = 8;
		constexpr int GRID_SIZE = CORE_CELLS + PADDING * 2;
		constexpr int CELL_COUNT = GRID_SIZE * GRID_SIZE;
		constexpr float BLEND_WIDTH = PADDING * CELL_SIZE;
		constexpr float DRAINAGE_EPSILON = 0.001f;
		constexpr int SOURCE_SPACING = 4;
		constexpr float LAKE_MIN_SURFACE_HEIGHT = WATER_HEIGHT + 4.0f;
		constexpr float LAKE_SHORE_DEPTH = 1.5f;
		constexpr float LAKE_FULL_DEPTH = 4.0f;
		constexpr float LAKE_SURFACE_CLEARANCE = 0.5f;

		struct AxisBlend
		{
			int low;
			int high;
			float amount;
		};

		AxisBlend calculateAxisBlend(int coordinate, int region)
		{
			float local = static_cast<float>(coordinate - region * REGION_SIZE);
			if (local < BLEND_WIDTH)
			{
				return {
					region - 1,
					region,
					glm::smoothstep(-BLEND_WIDTH, BLEND_WIDTH, local)
				};
			}
			if (local > REGION_SIZE - BLEND_WIDTH)
			{
				float distanceFromBoundary = local - REGION_SIZE;
				return {
					region,
					region + 1,
					glm::smoothstep(-BLEND_WIDTH, BLEND_WIDTH, distanceFromBoundary)
				};
			}
			return {region, region, 0.0f};
		}

		long long regionKey(int regionX, int regionZ)
		{
			return static_cast<long long>(
				(static_cast<unsigned long long>(static_cast<uint32_t>(regionX)) << 32)
				| static_cast<uint32_t>(regionZ));
		}
	}

	struct HydrologyGenerator::Region
	{
		int originX;
		int originZ;
		std::array<float, CELL_COUNT> riverMask{};
		std::array<float, CELL_COUNT> waterSurfaceHeight{};
		std::array<float, CELL_COUNT> riverFlow{};
	};

	HydrologyGenerator::HydrologyGenerator(long seed, HeightSampler heightSampler)
		: m_noiseGenerator(seed), m_heightSampler(std::move(heightSampler)) {}

	std::shared_ptr<HydrologyGenerator::Region> HydrologyGenerator::getRegion(int regionX, int regionZ)
	{
		long long key = regionKey(regionX, regionZ);
		std::lock_guard<std::mutex> lock(m_regionMutex);
		auto found = m_regions.find(key);
		if (found != m_regions.end()) return found->second;

		auto region = buildRegion(regionX, regionZ);
		m_regions.emplace(key, region);
		return region;
	}

	std::shared_ptr<HydrologyGenerator::Region> HydrologyGenerator::buildRegion(int regionX, int regionZ)
	{
		auto region = std::make_shared<Region>();
		region->originX = regionX * REGION_SIZE;
		region->originZ = regionZ * REGION_SIZE;

		std::array<float, CELL_COUNT> heights{};
		std::array<float, CELL_COUNT> drainageHeights{};
		std::array<float, CELL_COUNT> accumulation{};
		std::array<int, CELL_COUNT> receiver{};
		std::array<int, CELL_COUNT> order{};

		for (int gz = 0; gz < GRID_SIZE; ++gz)
		{
			for (int gx = 0; gx < GRID_SIZE; ++gx)
			{
				int index = gx + gz * GRID_SIZE;
				int worldX = region->originX + (gx - PADDING) * CELL_SIZE;
				int worldZ = region->originZ + (gz - PADDING) * CELL_SIZE;
				heights[index] = m_heightSampler(worldX, worldZ);
				drainageHeights[index] = heights[index];
				accumulation[index] = 1.0f;
				receiver[index] = -1;
				order[index] = index;
			}
		}

		using DrainageCell = std::pair<float, int>;
		std::priority_queue<DrainageCell, std::vector<DrainageCell>, std::greater<DrainageCell>> frontier;
		std::array<bool, CELL_COUNT> visited{};
		auto addBoundaryCell = [&](int gx, int gz)
		{
			int index = gx + gz * GRID_SIZE;
			if (visited[index]) return;
			visited[index] = true;
			frontier.emplace(drainageHeights[index], index);
		};
		for (int i = 0; i < GRID_SIZE; ++i)
		{
			addBoundaryCell(i, 0);
			addBoundaryCell(i, GRID_SIZE - 1);
			addBoundaryCell(0, i);
			addBoundaryCell(GRID_SIZE - 1, i);
		}

		while (!frontier.empty())
		{
			auto [currentHeight, index] = frontier.top();
			frontier.pop();
			int gx = index % GRID_SIZE;
			int gz = index / GRID_SIZE;
			for (int dz = -1; dz <= 1; ++dz)
			{
				for (int dx = -1; dx <= 1; ++dx)
				{
					if (dx == 0 && dz == 0) continue;
					int nx = gx + dx;
					int nz = gz + dz;
					if (nx < 0 || nx >= GRID_SIZE || nz < 0 || nz >= GRID_SIZE) continue;
					int neighbour = nx + nz * GRID_SIZE;
					if (visited[neighbour]) continue;

					visited[neighbour] = true;
					receiver[neighbour] = index;
					drainageHeights[neighbour] = std::max(
						drainageHeights[neighbour], currentHeight + DRAINAGE_EPSILON);
					frontier.emplace(drainageHeights[neighbour], neighbour);
				}
			}
		}

		std::sort(order.begin(), order.end(), [&](int a, int b) {
			return drainageHeights[a] > drainageHeights[b];
		});
		for (int index : order)
		{
			if (receiver[index] >= 0) accumulation[receiver[index]] += accumulation[index];
		}

		// Priority-flood raises enclosed depressions to their spill elevation.
		// That difference from the original terrain is the lake-basin depth.
		std::array<float, CELL_COUNT> lakeMask{};
		std::array<float, CELL_COUNT> lakeWaterHeight{};
		for (int index = 0; index < CELL_COUNT; ++index)
		{
			float fillDepth = drainageHeights[index] - heights[index];
			if (drainageHeights[index] <= LAKE_MIN_SURFACE_HEIGHT) continue;
			lakeMask[index] = glm::smoothstep(LAKE_SHORE_DEPTH, LAKE_FULL_DEPTH, fillDepth);
			lakeWaterHeight[index] = drainageHeights[index] - LAKE_SURFACE_CLEARANCE;
		}

		std::array<float, CELL_COUNT> sourceScores{};
		for (int gz = 0; gz < GRID_SIZE; ++gz)
		{
			for (int gx = 0; gx < GRID_SIZE; ++gx)
			{
				int index = gx + gz * GRID_SIZE;
				int worldX = region->originX + (gx - PADDING) * CELL_SIZE;
				int worldZ = region->originZ + (gz - PADDING) * CELL_SIZE;
				sourceScores[index] = m_noiseGenerator.noise2(
					worldX + 63017, worldZ - 47003, 0.006f, 2.0f, 0.5f, 2);
			}
		}

		std::array<float, CELL_COUNT> channelFlow{};
		std::array<float, CELL_COUNT> channelWaterHeight{};
		channelWaterHeight.fill(std::numeric_limits<float>::max());
		for (int gz = SOURCE_SPACING; gz < GRID_SIZE - SOURCE_SPACING; ++gz)
		{
			for (int gx = SOURCE_SPACING; gx < GRID_SIZE - SOURCE_SPACING; ++gx)
			{
				int source = gx + gz * GRID_SIZE;
				if (heights[source] <= WATER_HEIGHT + 12
					|| accumulation[source] > 10.0f
					|| sourceScores[source] < 0.58f)
				{
					continue;
				}

				bool localMaximum = true;
				for (int dz = -SOURCE_SPACING; dz <= SOURCE_SPACING && localMaximum; ++dz)
				{
					for (int dx = -SOURCE_SPACING; dx <= SOURCE_SPACING; ++dx)
					{
						if (dx == 0 && dz == 0) continue;
						int neighbour = gx + dx + (gz + dz) * GRID_SIZE;
						if (sourceScores[neighbour] > sourceScores[source])
						{
							localMaximum = false;
							break;
						}
					}
				}
				if (!localMaximum) continue;

				int current = source;
				float waterHeight = heights[source] - 1.5f;
				for (int step = 0; step < CELL_COUNT && current >= 0; ++step)
				{
					waterHeight = std::min(waterHeight, heights[current] - 1.5f);
					waterHeight = std::min(waterHeight, channelWaterHeight[current]);
					float downstreamStrength = glm::smoothstep(1.0f, 80.0f, accumulation[current]);
					channelFlow[current] = std::max(channelFlow[current], downstreamStrength);
					channelWaterHeight[current] = std::min(channelWaterHeight[current], waterHeight);
					if (heights[current] <= WATER_HEIGHT + 2) break;
					current = receiver[current];
				}
			}
		}

		for (int gz = 0; gz < GRID_SIZE; ++gz)
		{
			for (int gx = 0; gx < GRID_SIZE; ++gx)
			{
				int index = gx + gz * GRID_SIZE;
				float bestMask = 0.0f;
				float bestWaterHeight = static_cast<float>(WATER_HEIGHT);
				float bestFlow = 0.0f;
				constexpr int MAX_CHANNEL_RADIUS = 3;
				for (int dz = -MAX_CHANNEL_RADIUS; dz <= MAX_CHANNEL_RADIUS; ++dz)
				{
					for (int dx = -MAX_CHANNEL_RADIUS; dx <= MAX_CHANNEL_RADIUS; ++dx)
					{
						int sx = gx + dx;
						int sz = gz + dz;
						if (sx < 0 || sx >= GRID_SIZE || sz < 0 || sz >= GRID_SIZE) continue;
						int source = sx + sz * GRID_SIZE;
						float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));
						if (channelWaterHeight[source] == std::numeric_limits<float>::max()) continue;
						float channelRadius = utils::lerp(1.1f, 2.8f, channelFlow[source]);
						float mask = glm::clamp(1.0f - distance / channelRadius, 0.0f, 1.0f);
						if (mask > bestMask)
						{
							bestMask = mask;
							bestWaterHeight = channelWaterHeight[source];
							bestFlow = channelFlow[source];
						}
					}
				}
				region->riverMask[index] = bestMask;
				region->waterSurfaceHeight[index] = bestWaterHeight;
				region->riverFlow[index] = bestFlow;
				if (lakeMask[index] > region->riverMask[index])
				{
					region->riverMask[index] = lakeMask[index];
					region->waterSurfaceHeight[index] = lakeWaterHeight[index];
					region->riverFlow[index] = 0.0f;
				}
			}
		}

		return region;
	}

	HydrologySample HydrologyGenerator::sampleRegion(const Region& region, int x, int z) const
	{
		float gridX = static_cast<float>(x - region.originX) / CELL_SIZE + PADDING;
		float gridZ = static_cast<float>(z - region.originZ) / CELL_SIZE + PADDING;
		int x0 = glm::clamp(static_cast<int>(std::floor(gridX)), 0, GRID_SIZE - 2);
		int z0 = glm::clamp(static_cast<int>(std::floor(gridZ)), 0, GRID_SIZE - 2);
		float tx = glm::clamp(gridX - x0, 0.0f, 1.0f);
		float tz = glm::clamp(gridZ - z0, 0.0f, 1.0f);
		int nw = x0 + z0 * GRID_SIZE;
		int ne = nw + 1;
		int sw = nw + GRID_SIZE;
		int se = sw + 1;
		return {
			utils::bilerp(region.riverMask[nw], region.riverMask[ne],
				region.riverMask[sw], region.riverMask[se], tx, tz),
			utils::bilerp(region.waterSurfaceHeight[nw], region.waterSurfaceHeight[ne],
				region.waterSurfaceHeight[sw], region.waterSurfaceHeight[se], tx, tz),
			utils::bilerp(region.riverFlow[nw], region.riverFlow[ne],
				region.riverFlow[sw], region.riverFlow[se], tx, tz)
		};
	}

	HydrologySample HydrologyGenerator::sample(int x, int z)
	{
		int regionX = utils::floorDiv(x, REGION_SIZE);
		int regionZ = utils::floorDiv(z, REGION_SIZE);
		AxisBlend xBlend = calculateAxisBlend(x, regionX);
		AxisBlend zBlend = calculateAxisBlend(z, regionZ);
		std::array<int, 2> regionXs = {xBlend.low, xBlend.high};
		std::array<int, 2> regionZs = {zBlend.low, zBlend.high};
		std::array<float, 2> xWeights = {1.0f - xBlend.amount, xBlend.amount};
		std::array<float, 2> zWeights = {1.0f - zBlend.amount, zBlend.amount};

		float blendedMask = 0.0f;
		float weightedWaterHeight = 0.0f;
		float weightedFlow = 0.0f;
		for (int zi = 0; zi < 2; ++zi)
		{
			for (int xi = 0; xi < 2; ++xi)
			{
				float weight = xWeights[xi] * zWeights[zi];
				if (weight <= 0.0f) continue;
				auto region = getRegion(regionXs[xi], regionZs[zi]);
				HydrologySample regionSample = sampleRegion(*region, x, z);
				float riverWeight = weight * regionSample.mask;
				blendedMask += riverWeight;
				weightedWaterHeight += riverWeight * regionSample.waterSurfaceHeight;
				weightedFlow += riverWeight * regionSample.flow;
			}
		}

		return {
			blendedMask,
			blendedMask > 0.0001f
				? weightedWaterHeight / blendedMask
				: static_cast<float>(WATER_HEIGHT),
			blendedMask > 0.0001f ? weightedFlow / blendedMask : 0.0f
		};
	}
}
