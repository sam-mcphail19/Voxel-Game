#pragma once

#include <functional>
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

	class HydrologyGenerator
	{
	private:
		struct Region;

		using HeightSampler = std::function<float(int, int)>;

		NoiseGenerator m_noiseGenerator;
		HeightSampler m_heightSampler;
		std::mutex m_regionMutex;
		std::unordered_map<long long, std::shared_ptr<Region>> m_regions;

		std::shared_ptr<Region> getRegion(int regionX, int regionZ);
		std::shared_ptr<Region> buildRegion(int regionX, int regionZ);
		HydrologySample sampleRegion(const Region& region, int x, int z) const;

	public:
		HydrologyGenerator(long seed, HeightSampler heightSampler);

		HydrologySample sample(int x, int z);
	};
}
