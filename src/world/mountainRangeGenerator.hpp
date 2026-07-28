#pragma once

#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	struct MountainRangeSample
	{
		float rangeMask;
		float foothillMask;
		float coreMask;
		float peakMask;
		float passMask;
		float elevation;
	};

	class MountainRangeGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

	public:
		explicit MountainRangeGenerator(long seed);
		MountainRangeSample sample(int x, int z);
	};
}
