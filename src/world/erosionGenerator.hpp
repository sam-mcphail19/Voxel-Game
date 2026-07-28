#pragma once

#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	struct ErosionInput
	{
		float erosion;
		float humidity;
		float mountainRange;
		float mountainFoothills;
		float mountainCore;
		float valley;
		float canyon;
	};

	struct ErosionSample
	{
		float gullyMask;
		float talusMask;
		float depositionMask;
		float heightDelta;
		float densityScale;
	};

	class ErosionGenerator
	{
	private:
		NoiseGenerator m_noiseGenerator;

	public:
		explicit ErosionGenerator(long seed);
		ErosionSample sample(int x, int z, const ErosionInput& input);
	};
}
