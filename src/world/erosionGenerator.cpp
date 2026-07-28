#include "erosionGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <glm/common.hpp>
#include "../util/mathUtils.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr float WARP_SCALE = 0.0012f;
		constexpr float WARP_STRENGTH = 96.0f;
		constexpr float GULLY_SCALE = 0.0045f;
		constexpr float GULLY_DETAIL_SCALE = 0.015f;
		constexpr float DEPOSITION_SCALE = 0.0022f;

		constexpr float GULLY_START = 0.68f;
		constexpr float GULLY_FULL = 0.94f;
		constexpr float TALUS_RANGE_START = 0.18f;
		constexpr float TALUS_RANGE_FULL = 0.62f;
		constexpr float TALUS_CORE_FADE_START = 0.42f;
		constexpr float TALUS_CORE_FADE_END = 0.88f;
		constexpr float DEPOSITION_NOISE_START = 0.48f;
		constexpr float DEPOSITION_NOISE_FULL = 0.78f;

		constexpr float MIN_GULLY_DEPTH = 5.0f;
		constexpr float MAX_GULLY_DEPTH = 12.0f;
		constexpr float MAX_DEPOSITION_HEIGHT = 7.0f;
		constexpr float MAX_RIDGE_SHARPENING = 9.0f;
		constexpr float GULLY_DENSITY_REDUCTION = 0.42f;
		constexpr float TALUS_DENSITY_REDUCTION = 0.20f;
	}

	ErosionGenerator::ErosionGenerator(long seed)
		: m_noiseGenerator(seed ^ 0x45524f53L) {}

	ErosionSample ErosionGenerator::sample(
		int x, int z, const ErosionInput& input)
	{
		const float warpX = m_noiseGenerator.noise2(
			x + 23003, z - 41011, WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpZ = m_noiseGenerator.noise2(
			x - 37009, z + 17011, WARP_SCALE, 2.0f, 0.5f, 3) * 2.0f - 1.0f;
		const float warpedX = x + warpX * WARP_STRENGTH;
		const float warpedZ = z + warpZ * WARP_STRENGTH;

		const float channelNoise = m_noiseGenerator.noise2(
			warpedX, warpedZ, GULLY_SCALE, 2.0f, 0.52f, 4) * 2.0f - 1.0f;
		const float channel = 1.0f - std::abs(channelNoise);
		const float detail = m_noiseGenerator.noise2(
			warpedX + 53003.0f, warpedZ - 29009.0f,
			GULLY_DETAIL_SCALE, 2.0f, 0.48f, 3);

		const float ruggedness =
			1.0f - glm::smoothstep(0.38f, 0.82f, input.erosion);
		const float mountainRelief = glm::clamp(
			input.mountainRange * 0.55f + input.mountainCore * 0.75f,
			0.0f,
			1.0f);
		const float moistureResponse = utils::lerp(
			0.72f, 1.0f, glm::smoothstep(0.28f, 0.72f, input.humidity));
		const float gullyMask =
			glm::smoothstep(GULLY_START, GULLY_FULL, channel)
			* mountainRelief
			* ruggedness
			* moistureResponse
			* (1.0f - input.canyon * 0.65f);

		const float talusMask =
			glm::smoothstep(
				TALUS_RANGE_START, TALUS_RANGE_FULL, input.mountainFoothills)
			* (1.0f - glm::smoothstep(
				TALUS_CORE_FADE_START, TALUS_CORE_FADE_END, input.mountainCore))
			* glm::smoothstep(0.28f, 0.68f, input.erosion);

		const float depositionNoise = m_noiseGenerator.noise2(
			warpedX - 61001.0f, warpedZ + 47017.0f,
			DEPOSITION_SCALE, 2.0f, 0.5f, 3);
		const float depositionMask =
			talusMask
			* glm::smoothstep(
				DEPOSITION_NOISE_START, DEPOSITION_NOISE_FULL, depositionNoise)
			* (0.45f + input.valley * 0.55f);

		const float gullyDepth = utils::lerp(
			MIN_GULLY_DEPTH, MAX_GULLY_DEPTH,
			glm::clamp(input.mountainCore * 0.7f + detail * 0.3f, 0.0f, 1.0f));
		const float ridgeSharpening =
			input.mountainCore
			* ruggedness
			* (1.0f - gullyMask)
			* detail
			* MAX_RIDGE_SHARPENING;
		const float gullyCarvingStrength = gullyMask * gullyMask;
		const float heightDelta =
			ridgeSharpening
			- gullyCarvingStrength * gullyDepth
			+ depositionMask * MAX_DEPOSITION_HEIGHT;
		const float densityScale =
			1.0f
			- gullyMask * GULLY_DENSITY_REDUCTION
			- talusMask * TALUS_DENSITY_REDUCTION;

		return {
			gullyMask,
			talusMask,
			depositionMask,
			heightDelta,
			glm::clamp(densityScale, 0.35f, 1.0f)
		};
	}
}
