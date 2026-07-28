#include "terrainGenerator.hpp"
#include "worldProfiler.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <glm/common.hpp>
#include "../constants.hpp"
#include "../util/mathUtils.hpp"

namespace voxel_game::world
{
	namespace
	{
		struct NoiseParameters
		{
			float scale;
			float lacunarity;
			float persistence;
			int octaves;
		};

		constexpr NoiseParameters CONTINENTALNESS_NOISE{0.0010f, 2.0f, 0.5f, 5};
		constexpr NoiseParameters EROSION_NOISE{0.0016f, 2.0f, 0.5f, 4};
		// Climate changes at continental scale. Smaller scales here produce
		// larger, more coherent biome regions without changing terrain detail.
		constexpr NoiseParameters TEMPERATURE_NOISE{0.00028f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters HUMIDITY_NOISE{0.00036f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters WARP_NOISE{0.0018f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters RIDGE_NOISE{0.006f, 2.0f, 0.5f, 5};
		constexpr NoiseParameters TERRAIN_SELECTOR_NOISE{0.0011f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters PLATEAU_SELECTOR_NOISE{0.0014f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters HEIGHT_DETAIL_NOISE{0.018f, 2.0f, 0.48f, 4};
		constexpr NoiseParameters OCEAN_BASIN_NOISE{0.0024f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters SEAFLOOR_DETAIL_NOISE{0.010f, 2.0f, 0.45f, 3};
		constexpr NoiseParameters FORMATION_REGION_NOISE{0.0009f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters PLATEAU_NOISE{0.006f, 2.0f, 0.45f, 3};
		constexpr NoiseParameters DENSITY_NOISE{0.028f, 2.0f, 0.5f, 4};
		constexpr NoiseParameters FORMATION_NOISE{0.016f, 2.0f, 0.5f, 3};

		constexpr float TERRAIN_WARP_STRENGTH = 48.0f;
		// Keep terrain selected as ocean below the same continentalness range
		// used by BiomeSelector. This prevents dry land from being labelled Ocean.
		constexpr float OCEAN_CONTINENTALNESS = 0.42f;
		constexpr float INLAND_CONTINENTALNESS = 0.56f;
		constexpr float OCEAN_SHELF_BLEND_START = 0.40f;
		constexpr float OCEAN_SHELF_BLEND_END = 0.52f;
		constexpr float OCEAN_FLOOR_CLEARANCE = 5.0f;
		constexpr float OCEAN_DENSITY_STRENGTH = 1.25f;
		constexpr float DEEP_OCEAN_HEIGHT = WATER_HEIGHT - 24.0f;
		constexpr float COASTAL_SHELF_HEIGHT = WATER_HEIGHT - 10.0f;
		constexpr float DEEP_OCEAN_START = 0.10f;
		constexpr float DEEP_OCEAN_END = 0.30f;
		constexpr float OCEAN_BASIN_MAX_DEPTH = 18.0f;
		constexpr float SEAFLOOR_DETAIL_STRENGTH = 3.0f;
		constexpr float COAST_HEIGHT = WATER_HEIGHT + 5.0f;
		constexpr float HIGH_INLAND_HEIGHT = WATER_HEIGHT + 30.0f;
		constexpr float COAST_MASK_CENTER = 0.49f;
		constexpr float COAST_MASK_WIDTH = 0.10f;

		constexpr std::array<float, 5> BASE_OFFSETS{-2.0f, 0.0f, 12.0f, 18.0f, 8.0f};
		constexpr std::array<float, 5> HILL_STRENGTHS{2.0f, 8.0f, 12.0f, 3.0f, 10.0f};
		constexpr std::array<float, 5> DENSITY_STRENGTHS{1.0f, 2.0f, 4.0f, 1.5f, 11.0f};

		constexpr int PLATEAU_STEP_COUNT = 4;
		constexpr float PLATEAU_TRANSITION_WIDTH = 0.08f;
		constexpr float PLATEAU_MAX_HEIGHT = 26.0f;
		constexpr float MOUNTAIN_MAX_HEIGHT = 42.0f;
		constexpr float FORMATION_MAX_STRENGTH = 10.0f;
		constexpr float FORMATION_VERTICAL_SCALE = 0.45f;

		float sampleNoise2(NoiseGenerator& noise, float x, float z, const NoiseParameters& parameters)
		{
			return noise.noise2(
				x, z, parameters.scale, parameters.lacunarity,
				parameters.persistence, parameters.octaves);
		}

		float terrace(float value, int stepCount, float transitionWidth, float& cliffMask)
		{
			float scaled = glm::clamp(value, 0.0f, 0.9999f) * (stepCount - 1);
			float level = std::floor(scaled);
			float fraction = scaled - level;
			float transition = glm::smoothstep(
				0.5f - transitionWidth, 0.5f + transitionWidth, fraction);
			float distanceFromEdge = std::abs(fraction - 0.5f);
			cliffMask = 1.0f - glm::smoothstep(
				transitionWidth, transitionWidth * 2.5f, distanceFromEdge);
			return (level + transition) / static_cast<float>(stepCount - 1);
		}
	}

	TerrainGenerator::TerrainGenerator(long seed, PlainsWeightSampler plainsWeightSampler)
		: m_noiseGenerator(seed),
		  m_mountainRangeGenerator(seed),
		  m_valleyGenerator(seed),
		  m_erosionGenerator(seed),
		  m_plainsWeightSampler(std::move(plainsWeightSampler)) {}

	float TerrainGenerator::sampleContinentalness(int x, int z)
	{
		return sampleNoise2(m_noiseGenerator, x + 5555, z + 5555, CONTINENTALNESS_NOISE);
	}

	float TerrainGenerator::sampleErosion(int x, int z)
	{
		return sampleNoise2(m_noiseGenerator, x + 1111, z + 1111, EROSION_NOISE);
	}

	float TerrainGenerator::sampleTemperature(int x, int z)
	{
		return sampleNoise2(m_noiseGenerator, x, z, TEMPERATURE_NOISE);
	}

	float TerrainGenerator::sampleHumidity(int x, int z)
	{
		return sampleNoise2(m_noiseGenerator, x + 9999, z + 9999, HUMIDITY_NOISE);
	}

	struct TerrainGenerator::PipelineState
	{
		int x;
		int z;
		float warpedX = 0.0f;
		float warpedZ = 0.0f;
		float inland = 0.0f;
		float rugged = 0.0f;
		float rangeLand = 0.0f;
		float valleyLand = 0.0f;
		float valleyRelief = 0.0f;
		float plainsInfluence = 0.0f;
		float composedHeight = 0.0f;
		MountainRangeSample mountainRange{};
		ValleySample valley{};
		TerrainSample terrain{};
	};

	TerrainSample TerrainGenerator::sampleBase(int x, int z)
	{
		PipelineState state{x, z};
		sampleClimate(state);
		placeMacroLandforms(state);
		blendTerrainProfiles(state);
		composeBaseElevation(state);
		applyErosion(state);
		shapeOceanAndCoast(state);
		return finalizeTerrain(state);
	}

	void TerrainGenerator::sampleClimate(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		terrain.continentalness = sampleContinentalness(state.x, state.z);
		terrain.erosion = sampleErosion(state.x, state.z);
		terrain.temperature = sampleTemperature(state.x, state.z);
		terrain.humidity = sampleHumidity(state.x, state.z);

		const float warpX = sampleNoise2(
			m_noiseGenerator, state.x + 17011, state.z - 9041, WARP_NOISE) * 2.0f - 1.0f;
		const float warpZ = sampleNoise2(
			m_noiseGenerator, state.x - 12037, state.z + 22109, WARP_NOISE) * 2.0f - 1.0f;
		state.warpedX = state.x + warpX * TERRAIN_WARP_STRENGTH;
		state.warpedZ = state.z + warpZ * TERRAIN_WARP_STRENGTH;

		const float ridgeNoise = sampleNoise2(
			m_noiseGenerator, state.warpedX, state.warpedZ, RIDGE_NOISE) * 2.0f - 1.0f;
		terrain.ridge = 1.0f - std::abs(ridgeNoise);
		terrain.ridge *= terrain.ridge;
		state.inland = glm::smoothstep(0.50f, 0.82f, terrain.continentalness);
		state.rugged = 1.0f - glm::smoothstep(0.35f, 0.78f, terrain.erosion);
	}

	void TerrainGenerator::placeMacroLandforms(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		state.mountainRange = m_mountainRangeGenerator.sample(state.x, state.z);
		state.rangeLand = glm::smoothstep(0.47f, 0.66f, terrain.continentalness);
		terrain.mountainRangeMask = state.mountainRange.rangeMask * state.rangeLand;
		terrain.mountainFoothillMask = state.mountainRange.foothillMask * state.rangeLand;
		terrain.mountainCoreMask = state.mountainRange.coreMask * state.rangeLand;
		terrain.mountainPeakMask = state.mountainRange.peakMask * state.rangeLand;
		terrain.mountainPassMask = state.mountainRange.passMask * state.rangeLand;

		state.valley = m_valleyGenerator.sample(state.x, state.z);
		state.valleyLand = glm::smoothstep(0.49f, 0.68f, terrain.continentalness);
		state.valleyRelief = glm::clamp(
			terrain.mountainFoothillMask + terrain.mountainCoreMask * 0.65f,
			0.0f,
			1.0f);
		terrain.valleyMask =
			state.valley.valleyMask * state.valleyLand * state.valleyRelief;
		terrain.canyonMask =
			state.valley.canyonMask * state.valleyLand * state.valleyRelief;
	}

	void TerrainGenerator::blendTerrainProfiles(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		const float terrainSelector = sampleNoise2(
			m_noiseGenerator,
			state.warpedX - 18000.0f,
			state.warpedZ + 12000.0f,
			TERRAIN_SELECTOR_NOISE);
		const float plateauSelector = sampleNoise2(
			m_noiseGenerator,
			state.warpedX + 23000.0f,
			state.warpedZ + 19000.0f,
			PLATEAU_SELECTOR_NOISE);

		const float flatWeight = glm::smoothstep(0.58f, 0.84f, terrain.erosion);
		const float mountainWeight = std::max(
			state.inland * state.rugged
				* glm::smoothstep(0.62f, 0.84f, terrainSelector) * 0.55f,
			terrain.mountainRangeMask * utils::lerp(0.58f, 1.0f, state.rugged));
		const float plateauWeight =
			state.inland * glm::smoothstep(0.60f, 0.80f, plateauSelector)
			* (1.0f - glm::smoothstep(0.72f, 0.90f, state.rugged));
		const float highlandWeight = std::max(
			state.inland * state.rugged
				* glm::smoothstep(0.35f, 0.65f, terrainSelector)
				* (1.0f - mountainWeight),
			terrain.mountainFoothillMask
				* (1.0f - terrain.mountainCoreMask) * 0.85f);
		const float rollingWeight = 0.35f + (1.0f - flatWeight) * 0.35f;

		terrain.terrainWeights = {
			flatWeight, rollingWeight, highlandWeight, plateauWeight, mountainWeight
		};
		float weightSum = 0.0f;
		for (float weight : terrain.terrainWeights) weightSum += weight;
		for (float& weight : terrain.terrainWeights) weight /= weightSum;

		const float regionalMountainWeight =
			terrain.terrainWeights[static_cast<int>(TerrainType::Mountains)];
		terrain.mountainMask = std::max(
			regionalMountainWeight * glm::smoothstep(0.20f, 0.70f, terrain.ridge),
			terrain.mountainCoreMask);
		terrain.coastMask = 1.0f - glm::smoothstep(
			0.0f,
			COAST_MASK_WIDTH,
			std::abs(terrain.continentalness - COAST_MASK_CENTER));
	}

	void TerrainGenerator::composeBaseElevation(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		float baseHeight = 0.0f;
		if (terrain.continentalness < OCEAN_CONTINENTALNESS)
		{
			const float shelf = glm::smoothstep(
				0.05f, OCEAN_CONTINENTALNESS, terrain.continentalness);
			baseHeight = utils::lerp(DEEP_OCEAN_HEIGHT, COASTAL_SHELF_HEIGHT, shelf);
			const float deepOcean = 1.0f - glm::smoothstep(
				DEEP_OCEAN_START, DEEP_OCEAN_END, terrain.continentalness);
			const float basinNoise = sampleNoise2(
				m_noiseGenerator,
				state.warpedX + 53003.0f,
				state.warpedZ - 61001.0f,
				OCEAN_BASIN_NOISE);
			const float seafloorDetail = sampleNoise2(
				m_noiseGenerator,
				state.warpedX - 47017.0f,
				state.warpedZ + 43003.0f,
				SEAFLOOR_DETAIL_NOISE) * 2.0f - 1.0f;
			baseHeight -= deepOcean * basinNoise * OCEAN_BASIN_MAX_DEPTH;
			baseHeight += seafloorDetail * SEAFLOOR_DETAIL_STRENGTH;
		}
		else if (terrain.continentalness < INLAND_CONTINENTALNESS)
		{
			const float coast = glm::smoothstep(
				OCEAN_CONTINENTALNESS,
				INLAND_CONTINENTALNESS,
				terrain.continentalness);
			baseHeight = utils::lerp(COASTAL_SHELF_HEIGHT, COAST_HEIGHT, coast);
		}
		else
		{
			const float inlandHeight = glm::smoothstep(
				INLAND_CONTINENTALNESS, 0.82f, terrain.continentalness);
			baseHeight = utils::lerp(COAST_HEIGHT, HIGH_INLAND_HEIGHT, inlandHeight);
		}

		const float detail = sampleNoise2(
			m_noiseGenerator,
			state.warpedX + 7000.0f,
			state.warpedZ - 7000.0f,
			HEIGHT_DETAIL_NOISE) * 2.0f - 1.0f;
		float baseOffset = 0.0f;
		float hillStrength = 0.0f;
		for (size_t i = 0; i < terrain.terrainWeights.size(); ++i)
		{
			baseOffset += terrain.terrainWeights[i] * BASE_OFFSETS[i];
			hillStrength += terrain.terrainWeights[i] * HILL_STRENGTHS[i];
			terrain.densityStrength +=
				terrain.terrainWeights[i] * DENSITY_STRENGTHS[i];
		}
		hillStrength *= utils::lerp(0.52f, 1.05f, terrain.mountainRangeMask);
		terrain.densityStrength *=
			utils::lerp(0.68f, 1.10f, terrain.mountainCoreMask);

		state.plainsInfluence = m_plainsWeightSampler(terrain)
			* glm::smoothstep(0.48f, 0.78f, terrain.erosion);
		hillStrength *= utils::lerp(1.0f, 0.38f, state.plainsInfluence);
		terrain.densityStrength *=
			utils::lerp(1.0f, 0.55f, state.plainsInfluence);

		const float formationRegion = sampleNoise2(
			m_noiseGenerator,
			state.warpedX + 41000.0f,
			state.warpedZ - 35000.0f,
			FORMATION_REGION_NOISE);
		const float formationLand =
			glm::smoothstep(0.54f, 0.72f, terrain.continentalness);
		const float formationRuggedness =
			1.0f - glm::smoothstep(0.52f, 0.78f, terrain.erosion);
		terrain.formationMask = formationLand * formationRuggedness
			* glm::smoothstep(0.68f, 0.86f, formationRegion)
			* (1.0f - state.plainsInfluence);
		terrain.formationStrength =
			terrain.formationMask * FORMATION_MAX_STRENGTH;

		const float plateauNoise = sampleNoise2(
			m_noiseGenerator,
			state.warpedX - 4000.0f,
			state.warpedZ + 4000.0f,
			PLATEAU_NOISE);
		float localPlateauCliff = 0.0f;
		const float plateauSteps = terrace(
			plateauNoise,
			PLATEAU_STEP_COUNT,
			PLATEAU_TRANSITION_WIDTH,
			localPlateauCliff);
		const float plateauWeight =
			terrain.terrainWeights[static_cast<int>(TerrainType::Plateaus)];
		terrain.plateauCliffMask = localPlateauCliff * plateauWeight
			* utils::lerp(1.0f, 0.25f, state.plainsInfluence);
		const float plateauHeight =
			plateauSteps * PLATEAU_MAX_HEIGHT * plateauWeight
			* utils::lerp(1.0f, 0.30f, state.plainsInfluence);
		const float plateauTopFlattening =
			1.0f - plateauWeight * (1.0f - terrain.plateauCliffMask) * 0.75f;
		const float hills = detail * hillStrength * plateauTopFlattening;
		terrain.densityStrength += terrain.plateauCliffMask * 4.0f;
		const float mountains =
			terrain.ridge * terrain.mountainMask * MOUNTAIN_MAX_HEIGHT
			* utils::lerp(1.0f, 0.20f, state.plainsInfluence);
		const float rangeHeight =
			state.mountainRange.elevation
			* state.rangeLand
			* utils::lerp(0.55f, 1.0f, state.rugged)
			* utils::lerp(1.0f, 0.35f, state.plainsInfluence);

		const float desiredValleyCarving =
			state.valley.carvingDepth * state.valleyLand * state.valleyRelief;
		const float uncarvedHeight =
			baseHeight + baseOffset + hills + plateauHeight + mountains + rangeHeight;
		const float availableCarving = std::max(
			0.0f, uncarvedHeight - (WATER_HEIGHT + 5.0f));
		const float valleyCarving = availableCarving > 0.0f
			? availableCarving
				* (1.0f - std::exp(-desiredValleyCarving / availableCarving))
			: 0.0f;
		state.composedHeight = uncarvedHeight - valleyCarving;
		terrain.densityStrength *=
			utils::lerp(1.0f, 0.62f, terrain.valleyMask);
		terrain.formationStrength *= 1.0f - terrain.canyonMask * 0.75f;
	}

	void TerrainGenerator::applyErosion(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		const ErosionSample erosion = m_erosionGenerator.sample(
			state.x,
			state.z,
			ErosionInput{
				terrain.erosion,
				terrain.humidity,
				terrain.mountainRangeMask,
				terrain.mountainFoothillMask,
				terrain.mountainCoreMask,
				terrain.valleyMask,
				terrain.canyonMask
			});
		terrain.erosionGullyMask = erosion.gullyMask;
		terrain.talusMask = erosion.talusMask;
		terrain.depositionMask = erosion.depositionMask;
		terrain.erosionHeightDelta = erosion.heightDelta;
		state.composedHeight += erosion.heightDelta;
		terrain.densityStrength *= erosion.densityScale;
		terrain.formationStrength *= 1.0f - erosion.gullyMask * 0.75f;
	}

	void TerrainGenerator::shapeOceanAndCoast(PipelineState& state)
	{
		TerrainSample& terrain = state.terrain;
		const float shelfLandBlend = glm::smoothstep(
			OCEAN_SHELF_BLEND_START,
			OCEAN_SHELF_BLEND_END,
			terrain.continentalness);
		const float oceanInfluence = 1.0f - shelfLandBlend;
		const float offshoreDensity = std::min(
			terrain.densityStrength, OCEAN_DENSITY_STRENGTH);
		terrain.densityStrength = utils::lerp(
			offshoreDensity, terrain.densityStrength, shelfLandBlend);
		terrain.formationMask *= shelfLandBlend;
		terrain.formationStrength *= shelfLandBlend;

		const float maximumOceanFloor =
			WATER_HEIGHT - OCEAN_FLOOR_CLEARANCE - terrain.densityStrength;
		const float constrainedOceanFloor =
			std::min(state.composedHeight, maximumOceanFloor);
		state.composedHeight = utils::lerp(
			state.composedHeight, constrainedOceanFloor, oceanInfluence);
	}

	TerrainSample TerrainGenerator::finalizeTerrain(PipelineState& state)
	{
		state.terrain.height = glm::clamp(
			state.composedHeight,
			static_cast<float>(MIN_WORLD_GEN_HEIGHT),
			static_cast<float>(MAX_WORLD_GEN_HEIGHT));
		return state.terrain;
	}

	float TerrainGenerator::sampleDensity(const TerrainSample& terrain, int x, int y, int z)
	{
		float baseDensity = terrain.height - static_cast<float>(y);
		float maximumDisplacement = terrain.densityStrength + terrain.formationStrength;
		if (baseDensity < -maximumDisplacement || baseDensity > maximumDisplacement)
		{
			WorldProfiler::instance().increment(ProfileCounter::TerrainDensityFarRejections);
			return baseDensity;
		}

		float density = baseDensity;
		if (baseDensity <= maximumDisplacement)
		{
			WorldProfiler::instance().increment(ProfileCounter::Terrain3dNoiseSamples);
			float densityNoise = m_noiseGenerator.noise3(
				x + 3107, y - 1973, z + 7919,
				DENSITY_NOISE.scale, DENSITY_NOISE.lacunarity,
				DENSITY_NOISE.persistence, DENSITY_NOISE.octaves) * 2.0f - 1.0f;
			density += densityNoise * terrain.densityStrength;
		}
		if (terrain.formationStrength > 0.01f)
		{
			WorldProfiler::instance().increment(ProfileCounter::Terrain3dNoiseSamples);
			float formationNoise = m_noiseGenerator.noise3(
				x - 15001.0f, y * FORMATION_VERTICAL_SCALE + 9000.0f, z + 19001.0f,
				FORMATION_NOISE.scale, FORMATION_NOISE.lacunarity,
				FORMATION_NOISE.persistence, FORMATION_NOISE.octaves) * 2.0f - 1.0f;
			density += formationNoise * terrain.formationStrength;
		}
		return density;
	}
}
