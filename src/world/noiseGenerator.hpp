#pragma once

#include <math.h>
#include "../vendor/OpenSimplex2S.hpp"

namespace voxel_game::world
{
	class NoiseGenerator
	{
	private:
		OpenSimplex2S* m_simplex;

	public:
		NoiseGenerator(long seed);
		// Returns a value between 0 and 1
		float noise2(int x, int y, float scale, float lacunarity, float persistance, int octaves);
	};
}