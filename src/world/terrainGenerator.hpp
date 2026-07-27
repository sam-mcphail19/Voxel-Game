#pragma once

#include <array>
#include <functional>
#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	enum class TerrainType
	{
		Flatlands,
		RollingHills,
		Highlands,
		Plateaus,
		Mountains
	};

	struct TerrainSample
	{
		float continentalness;
		float erosion;
		float temperature;
		float humidity;
		float ridge;
		float mountainMask;
		float coastMask;
		float plateauCliffMask;
		float formationMask;
		float formationStrength;
		float riverMask;
		float riverSurfaceHeight;
		float riverFlow;
		float densityStrength;
		std::array<float, 5> terrainWeights;
		float height;
	};

	class TerrainGenerator
	{
	public:
		using PlainsWeightSampler = std::function<float(const TerrainSample&)>;

	private:
		NoiseGenerator m_noiseGenerator;
		PlainsWeightSampler m_plainsWeightSampler;

	public:
		TerrainGenerator(long seed, PlainsWeightSampler plainsWeightSampler);

		float sampleContinentalness(int x, int z);
		float sampleErosion(int x, int z);
		float sampleTemperature(int x, int z);
		float sampleHumidity(int x, int z);
		TerrainSample sampleBase(int x, int z);
		float sampleDensity(const TerrainSample& terrain, int x, int y, int z);
	};
}
