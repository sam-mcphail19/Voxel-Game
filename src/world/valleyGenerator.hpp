#pragma once

#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	struct ValleySample
	{
		float valleyMask;
		float canyonMask;
		float carvingDepth;
	};

	class ValleyGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

	public:
		explicit ValleyGenerator(long seed);
		ValleySample sample(int x, int z);
	};
}
