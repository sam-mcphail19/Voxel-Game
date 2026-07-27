#pragma once

#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	class CaveGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

		float sampleTunnelDensity(int x, int y, int z, float surfaceHeight);
		float sampleRavineDensity(int x, int y, int z, float surfaceHeight);

	public:
		explicit CaveGenerator(long seed);

		// Returns a signed density: negative values carve air and positive
		// values leave the terrain density unchanged.
		float sampleDensity(int x, int y, int z, float surfaceHeight);
	};
}
