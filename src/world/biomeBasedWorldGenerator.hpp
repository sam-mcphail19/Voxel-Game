#pragma once

#include <algorithm>
#include <array>
#include <unordered_map>
#include "worldGenerator.hpp"
#include "biome.hpp"
#include "chunk.hpp"
#include "noiseGenerator.hpp"
#include "hydrologyGenerator.hpp"
#include "caveGenerator.hpp"
#include "terrainGenerator.hpp"
#include "biomeSelector.hpp"
#include "surfaceFeatureGenerator.hpp"
#include "../common/threadSafeMap.hpp"
#include "../util/mathUtils.hpp"
#include "../util/log.hpp"

namespace voxel_game::world
{
	struct TerrainDebugSample
	{
		float continentalness;
		float erosion;
		float temperature;
		float humidity;
		float ridge;
		float mountainRanges;
		float mountainFoothills;
		float mountainCores;
		float mountainPeaks;
		float mountainPasses;
		float valleys;
		float canyons;
		float erosionGullies;
		float talus;
		float deposition;
		float coast;
		float slope;
		float plateauCliffs;
		float formations;
		float rivers;
		float riverFlow;
		float height;
		float densityStrength;
		std::array<float, 5> terrainWeights;
		TerrainType dominantTerrain;
		BiomeType biome;
		int surfaceHeight;
		BlockTypeId surfaceBlock;
	};

	class BiomeBasedWorldGenerator : public WorldGenerator
	{
	private:
		BiomeSelector m_biomeSelector;
		TerrainGenerator m_terrainGenerator;
		HydrologyGenerator m_hydrologyGenerator;
		CaveGenerator m_caveGenerator;
		SurfaceFeatureGenerator m_surfaceFeatureGenerator;
		ThreadSafeMap<int, std::unordered_map<int, int>> m_heightMap;

		TerrainSample sampleTerrain(int x, int z);
		float calculateDensity(const TerrainSample& terrain, int x, int y, int z);
		float calculateDensity(const TerrainSample& terrain, const CaveColumnSample& caveColumn, int x, int y, int z);
		bool isWater(
			const TerrainSample& terrain,
			int surfaceHeight,
			int y
		) const;
		int calculateSurfaceHeight(const TerrainSample& terrain, int x, int z);
		int calculateSurfaceHeight(const TerrainSample& terrain, const CaveColumnSample& caveColumn, int x, int z);
		float calculateSlope(int x, int z);
		BlockTypeId getSurfaceBlockType(const Biome* biome, const TerrainSample& terrain, float slope, int x, int z, int surfaceHeight);
		std::vector<BiomeWeight> buildWeights(const TerrainSample& terrain);
		int calculateHeight(int x, int z);
		const Biome* selectBiome(
			std::vector<BiomeWeight> weights,
			const TerrainSample& terrain,
			int x,
			int z);

	public:
		BiomeBasedWorldGenerator(long seed);

		float calcContinentalness(int x, int z);
		float calcErosion(int x, int z);
		float calcTemperature(int x, int z);
		float calcHumidity(int x, int z);
		TerrainDebugSample getTerrainDebugSample(int x, int z);
		static BlockTypeId selectSurfaceBlock(BlockTypeId biomeSurfaceBlock, float coastMask, float slope, int surfaceHeight);

		std::vector<BiomeWeight> buildWeights(int x, int z);
		std::vector<BiomeWeight> normalizeWeights(std::vector<BiomeWeight> weights);
		int getHeight(int x, int z);
		BlockTypeId getBlockType(BlockPos pos);
		void generateChunkData(Chunk &chunk);
	};
}
