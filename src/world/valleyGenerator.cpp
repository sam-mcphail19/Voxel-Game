#include "valleyGenerator.hpp"

#include <cmath>
#include <glm/common.hpp>
#include "../util/mathUtils.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr float VALLEY_SPINE_SCALE = 0.00055f;
		constexpr float VALLEY_REGION_SCALE = 0.00022f;
		constexpr float VALLEY_WARP_SCALE = 0.00030f;
		constexpr float VALLEY_WARP_STRENGTH = 720.0f;
		constexpr float CANYON_DETAIL_SCALE = 0.0014f;

		constexpr float VALLEY_START = 0.58f;
		constexpr float VALLEY_FULL = 0.88f;
		constexpr float CANYON_START = 0.82f;
		constexpr float CANYON_FULL = 0.965f;
		constexpr float REGION_START = 0.40f;
		constexpr float REGION_FULL = 0.66f;

		constexpr float VALLEY_DEPTH = 42.0f;
		constexpr float CANYON_DEPTH = 58.0f;
		constexpr float CANYON_DEPTH_VARIATION = 24.0f;
	}

	ValleyGenerator::ValleyGenerator(long seed)
		: m_noiseGenerator(seed ^ 0x56414c4cL) {}

	ValleySample ValleyGenerator::sample(int x, int z)
	{
		const float warpX = m_noiseGenerator.noise2(
			x + 37009, z - 17011, VALLEY_WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpZ = m_noiseGenerator.noise2(
			x - 23003, z + 41011, VALLEY_WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpedX = x + warpX * VALLEY_WARP_STRENGTH;
		const float warpedZ = z + warpZ * VALLEY_WARP_STRENGTH;

		const float spineNoise = m_noiseGenerator.noise2(
			warpedX, warpedZ, VALLEY_SPINE_SCALE, 2.0f, 0.52f, 4) * 2.0f - 1.0f;
		const float spine = 1.0f - std::abs(spineNoise);
		const float region = glm::smoothstep(
			REGION_START,
			REGION_FULL,
			m_noiseGenerator.noise2(
				warpedX + 61001.0f, warpedZ - 53003.0f,
				VALLEY_REGION_SCALE, 2.0f, 0.5f, 3));

		const float valleyMask =
			glm::smoothstep(VALLEY_START, VALLEY_FULL, spine) * region;
		const float canyonShape =
			glm::smoothstep(CANYON_START, CANYON_FULL, spine);
		const float canyonRegion = glm::smoothstep(
			0.50f,
			0.76f,
			m_noiseGenerator.noise2(
				warpedX - 43003.0f, warpedZ + 29009.0f,
				CANYON_DETAIL_SCALE, 2.0f, 0.5f, 3));
		const float canyonMask = canyonShape * region * canyonRegion;
		const float depthVariation = m_noiseGenerator.noise2(
			warpedX + 11027.0f, warpedZ + 71011.0f,
			CANYON_DETAIL_SCALE, 2.0f, 0.48f, 3);
		const float carvingDepth =
			valleyMask * VALLEY_DEPTH
			+ canyonMask * (CANYON_DEPTH + depthVariation * CANYON_DEPTH_VARIATION);

		return { valleyMask, canyonMask, carvingDepth };
	}
}
