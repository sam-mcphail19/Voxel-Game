#include "mountainRangeGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <glm/common.hpp>
#include "../util/mathUtils.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr float MACRO_REGION_SCALE = 0.00018f;
		constexpr float RANGE_SPINE_SCALE = 0.00042f;
		constexpr float RANGE_WARP_SCALE = 0.00032f;
		constexpr float RANGE_WARP_STRENGTH = 620.0f;
		constexpr float RIDGE_DETAIL_SCALE = 0.0024f;
		constexpr float PEAK_VARIATION_SCALE = 0.00085f;
		constexpr float PASS_SCALE = 0.00135f;

		constexpr float FOOTHILL_START = 0.43f;
		constexpr float FOOTHILL_FULL = 0.72f;
		constexpr float CORE_START = 0.70f;
		constexpr float CORE_FULL = 0.91f;
		constexpr float REGION_START = 0.46f;
		constexpr float REGION_FULL = 0.64f;
		constexpr float PASS_START = 0.76f;
		constexpr float PASS_FULL = 0.91f;

		constexpr float FOOTHILL_HEIGHT = 34.0f;
		constexpr float RANGE_SHOULDER_HEIGHT = 62.0f;
		constexpr float CORE_PEAK_HEIGHT = 132.0f;
		constexpr float FOOTHILL_PROFILE_EXPONENT = 0.90f;
		constexpr float CORE_PROFILE_EXPONENT = 1.45f;
		constexpr float SUMMIT_DETAIL_BASE = 14.0f;
		constexpr float SUMMIT_DETAIL_STRENGTH = 30.0f;
		constexpr float PASS_DEPTH = 0.72f;
	}

	MountainRangeGenerator::MountainRangeGenerator(long seed)
		: m_noiseGenerator(seed ^ 0x4d52474cL) {}

	MountainRangeSample MountainRangeGenerator::sample(int x, int z)
	{
		const float warpX = m_noiseGenerator.noise2(
			x + 17041, z - 29009, RANGE_WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpZ = m_noiseGenerator.noise2(
			x - 31013, z + 11027, RANGE_WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpedX = x + warpX * RANGE_WARP_STRENGTH;
		const float warpedZ = z + warpZ * RANGE_WARP_STRENGTH;

		const float spineNoise = m_noiseGenerator.noise2(
			warpedX, warpedZ, RANGE_SPINE_SCALE, 2.0f, 0.52f, 4) * 2.0f - 1.0f;
		const float spine = 1.0f - std::abs(spineNoise);
		const float region = glm::smoothstep(
			REGION_START,
			REGION_FULL,
			m_noiseGenerator.noise2(
				warpedX + 53003.0f, warpedZ - 47017.0f,
				MACRO_REGION_SCALE, 2.0f, 0.5f, 3));

		const float foothillMask =
			glm::smoothstep(FOOTHILL_START, FOOTHILL_FULL, spine) * region;
		const float coreMask =
			glm::smoothstep(CORE_START, CORE_FULL, spine) * region;

		const float detailNoise = m_noiseGenerator.noise2(
			warpedX - 13001.0f, warpedZ + 19001.0f,
			RIDGE_DETAIL_SCALE, 2.0f, 0.48f, 4) * 2.0f - 1.0f;
		const float ridgeDetail = std::pow(1.0f - std::abs(detailNoise), 1.65f);
		const float peakVariation = m_noiseGenerator.noise2(
			warpedX + 23003.0f, warpedZ + 37009.0f,
			PEAK_VARIATION_SCALE, 2.0f, 0.5f, 3);
		const float peakMask = coreMask
			* glm::smoothstep(0.28f, 0.92f, ridgeDetail)
			* utils::lerp(0.65f, 1.0f, peakVariation);

		const float passNoise = m_noiseGenerator.noise2(
			warpedX - 61001.0f, warpedZ - 41011.0f,
			PASS_SCALE, 2.0f, 0.5f, 3);
		const float passMask =
			coreMask * glm::smoothstep(PASS_START, PASS_FULL, passNoise);

		const float foothillProgress = glm::clamp(
			(spine - FOOTHILL_START) / (1.0f - FOOTHILL_START),
			0.0f,
			1.0f);
		const float coreProgress = glm::clamp(
			(spine - CORE_START) / (1.0f - CORE_START),
			0.0f,
			1.0f);
		const float foothillProfile =
			std::pow(foothillProgress, FOOTHILL_PROFILE_EXPONENT) * region;
		const float coreProfile =
			std::pow(coreProgress, CORE_PROFILE_EXPONENT) * region;
		const float summitDetail = coreProfile
			* (SUMMIT_DETAIL_BASE + ridgeDetail * SUMMIT_DETAIL_STRENGTH);
		float elevation =
			foothillProfile * FOOTHILL_HEIGHT
			+ coreProfile * RANGE_SHOULDER_HEIGHT
			+ summitDetail
			+ peakMask * ridgeDetail * CORE_PEAK_HEIGHT;
		elevation *= 1.0f - passMask * PASS_DEPTH;

		return {
			foothillMask,
			foothillMask,
			coreMask,
			peakMask,
			passMask,
			elevation
		};
	}
}
