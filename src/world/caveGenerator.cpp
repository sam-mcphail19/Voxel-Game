#include "caveGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <glm/common.hpp>

namespace voxel_game::world
{
	namespace
	{
		constexpr float SURFACE_CAVE_BUFFER = 14.0f;
		constexpr int BEDROCK_CAVE_BUFFER = 10;
		constexpr float TUNNEL_SCALE = 0.018f;
		constexpr float TUNNEL_RADIUS = 0.105f;
		constexpr float TUNNEL_EDGE_SHARPNESS = 42.0f;
		constexpr float CHAMBER_SCALE = 0.007f;
		constexpr float CHAMBER_THRESHOLD = 0.79f;
		constexpr float CHAMBER_EDGE_SHARPNESS = 28.0f;

		constexpr float RAVINE_REGION_SCALE = 0.0035f;
		constexpr float RAVINE_REGION_THRESHOLD = 0.78f;
		constexpr float RAVINE_REGION_EDGE_SHARPNESS = 35.0f;
		constexpr float RAVINE_PATH_SCALE = 0.006f;
		constexpr float RAVINE_HALF_WIDTH = 0.055f;
		constexpr float RAVINE_EDGE_SHARPNESS = 75.0f;
		constexpr float RAVINE_MAX_DEPTH = 32.0f;
		constexpr float RAVINE_FLOOR_SHARPNESS = 0.65f;
	}

	CaveGenerator::CaveGenerator(long seed)
		: m_noiseGenerator(seed) {}

	float CaveGenerator::sampleTunnelDensity(int x, int y, int z, float surfaceHeight)
	{
		float depth = surfaceHeight - static_cast<float>(y);

		// Keep entrances uncommon and preserve a solid foundation above
		// bedrock. The fade is expressed as added density so cave surfaces
		// remain smooth instead of being abruptly clipped.
		float boundaryDensity = 0.0f;
		if (depth < SURFACE_CAVE_BUFFER)
		{
			boundaryDensity += (SURFACE_CAVE_BUFFER - depth) * 1.5f;
		}
		if (y < BEDROCK_CAVE_BUFFER)
		{
			boundaryDensity += static_cast<float>(BEDROCK_CAVE_BUFFER - y) * 2.0f;
		}

		float tunnelA = m_noiseGenerator.noise3(
			x + 5107, y - 2903, z + 11027, TUNNEL_SCALE, 2.0f, 0.5f, 2) - 0.5f;
		float tunnelB = m_noiseGenerator.noise3(
			x - 17011, y + 7307, z - 13001, TUNNEL_SCALE, 2.0f, 0.5f, 2) - 0.5f;
		float tunnelDistance = std::abs(tunnelA) + std::abs(tunnelB);
		float tunnelDensity = (tunnelDistance - TUNNEL_RADIUS) * TUNNEL_EDGE_SHARPNESS;

		// A lower-frequency field occasionally opens tunnels into rooms without
		// turning the underground into one continuous hollow volume.
		float chamberNoise = m_noiseGenerator.noise3(
			x + 31001, y - 19001, z + 23003, CHAMBER_SCALE, 2.0f, 0.5f, 2);
		float chamberDensity =
			(CHAMBER_THRESHOLD - chamberNoise) * CHAMBER_EDGE_SHARPNESS;

		return std::min(tunnelDensity, chamberDensity) + boundaryDensity;
	}

	float CaveGenerator::sampleRavineDensity(int x, int y, int z, float surfaceHeight)
	{
		float regionNoise = m_noiseGenerator.noise2(
			x - 41009, z + 37003, RAVINE_REGION_SCALE, 2.0f, 0.5f, 2);
		float regionDensity =
			(RAVINE_REGION_THRESHOLD - regionNoise) * RAVINE_REGION_EDGE_SHARPNESS;

		float pathNoise = m_noiseGenerator.noise2(
			x + 19001, z - 29009, RAVINE_PATH_SCALE, 2.0f, 0.5f, 2) * 2.0f - 1.0f;
		float pathDensity =
			(std::abs(pathNoise) - RAVINE_HALF_WIDTH) * RAVINE_EDGE_SHARPNESS;

		float depth = surfaceHeight - static_cast<float>(y);
		float floorDensity = (depth - RAVINE_MAX_DEPTH) * RAVINE_FLOOR_SHARPNESS;

		// max() intersects the regional, path, and depth constraints. A ravine
		// exists only where all three signed fields are negative.
		return std::max({regionDensity, pathDensity, floorDensity});
	}

	float CaveGenerator::sampleDensity(int x, int y, int z, float surfaceHeight)
	{
		return std::min(
			sampleTunnelDensity(x, y, z, surfaceHeight),
			sampleRavineDensity(x, y, z, surfaceHeight));
	}
}
