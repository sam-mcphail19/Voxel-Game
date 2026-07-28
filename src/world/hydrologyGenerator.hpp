#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	struct HydrologySample
	{
		float mask;
		float waterSurfaceHeight;
		float flow;
	};

	struct HydrologyTerrainSample
	{
		float height;
		float valleyMask;
		float canyonMask;
	};

	class HydrologyGenerator
	{
	private:
		struct Region;

		using TerrainSampler = std::function<HydrologyTerrainSample(int, int)>;

		NoiseGenerator m_noiseGenerator;
		TerrainSampler m_terrainSampler;
		std::mutex m_regionMutex;
		using RegionFuture = std::shared_future<std::shared_ptr<Region>>;
		std::unordered_map<long long, RegionFuture> m_regions;

		std::shared_ptr<Region> getRegion(int regionX, int regionZ);
		std::shared_ptr<Region> buildRegion(int regionX, int regionZ);
		HydrologySample sampleRegion(const Region& region, int x, int z) const;

	public:
		HydrologyGenerator(long seed, TerrainSampler terrainSampler);

		HydrologySample sample(int x, int z);
	};
}
