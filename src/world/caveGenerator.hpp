#pragma once

#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	struct CaveColumnSample
	{
		float ravineHorizontalDensity;
	};

	class CaveGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

		float sampleTunnelDensity(int x, int y, int z, float surfaceHeight);
		float sampleRavineDensity(const CaveColumnSample& column, int y, float surfaceHeight);

	public:
		explicit CaveGenerator(long seed);
		CaveColumnSample sampleColumn(int x, int z);

		// Returns a signed density: negative values carve air and positive
		// values leave the terrain density unchanged.
		float sampleDensity(int x, int y, int z, float surfaceHeight);
		float sampleDensity(const CaveColumnSample& column, int x, int y, int z, float surfaceHeight);
	};
}
