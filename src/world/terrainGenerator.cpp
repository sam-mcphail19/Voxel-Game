#include "terrainGenerator.hpp"

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
		constexpr NoiseParameters TEMPERATURE_NOISE{0.0005f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters HUMIDITY_NOISE{0.00075f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters WARP_NOISE{0.0018f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters RIDGE_NOISE{0.006f, 2.0f, 0.5f, 5};
		constexpr NoiseParameters TERRAIN_SELECTOR_NOISE{0.0011f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters PLATEAU_SELECTOR_NOISE{0.0014f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters HEIGHT_DETAIL_NOISE{0.018f, 2.0f, 0.48f, 4};
		constexpr NoiseParameters FORMATION_REGION_NOISE{0.0009f, 2.0f, 0.5f, 3};
		constexpr NoiseParameters PLATEAU_NOISE{0.006f, 2.0f, 0.45f, 3};
		constexpr NoiseParameters DENSITY_NOISE{0.028f, 2.0f, 0.5f, 4};
		constexpr NoiseParameters FORMATION_NOISE{0.016f, 2.0f, 0.5f, 3};

		constexpr float TERRAIN_WARP_STRENGTH = 48.0f;
		constexpr float OCEAN_CONTINENTALNESS = 0.32f;
		constexpr float INLAND_CONTINENTALNESS = 0.52f;
		constexpr float DEEP_OCEAN_HEIGHT = WATER_HEIGHT - 24.0f;
		constexpr float COASTAL_SHELF_HEIGHT = WATER_HEIGHT - 10.0f;
		constexpr float COAST_HEIGHT = WATER_HEIGHT + 5.0f;
		constexpr float HIGH_INLAND_HEIGHT = WATER_HEIGHT + 30.0f;
		constexpr float COAST_MASK_CENTER = 0.445f;
		constexpr float COAST_MASK_WIDTH = 0.06f;

		constexpr std::array<float, 5> BASE_OFFSETS{-2.0f, 0.0f, 12.0f, 18.0f, 8.0f};
		constexpr std::array<float, 5> HILL_STRENGTHS{2.0f, 8.0f, 12.0f, 3.0f, 10.0f};
		constexpr std::array<float, 5> DENSITY_STRENGTHS{1.0f, 2.0f, 4.0f, 1.5f, 11.0f};

		constexpr int PLATEAU_STEP_COUNT = 4;
		constexpr float PLATEAU_TRANSITION_WIDTH = 0.08f;
		constexpr float PLATEAU_MAX_HEIGHT = 26.0f;
		constexpr float MOUNTAIN_MAX_HEIGHT = 72.0f;
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
		: m_noiseGenerator(seed), m_plainsWeightSampler(std::move(plainsWeightSampler)) {}

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

	TerrainSample TerrainGenerator::sampleBase(int x, int z)
	{
		TerrainSample result{};
		result.continentalness = sampleContinentalness(x, z);
		result.erosion = sampleErosion(x, z);
		result.temperature = sampleTemperature(x, z);
		result.humidity = sampleHumidity(x, z);

		float warpX = sampleNoise2(m_noiseGenerator, x + 17011, z - 9041, WARP_NOISE) * 2.0f - 1.0f;
		float warpZ = sampleNoise2(m_noiseGenerator, x - 12037, z + 22109, WARP_NOISE) * 2.0f - 1.0f;
		float warpedX = x + warpX * TERRAIN_WARP_STRENGTH;
		float warpedZ = z + warpZ * TERRAIN_WARP_STRENGTH;

		float ridgeNoise = sampleNoise2(m_noiseGenerator, warpedX, warpedZ, RIDGE_NOISE) * 2.0f - 1.0f;
		result.ridge = 1.0f - std::abs(ridgeNoise);
		result.ridge *= result.ridge;

		float inland = glm::smoothstep(0.50f, 0.82f, result.continentalness);
		float rugged = 1.0f - glm::smoothstep(0.35f, 0.78f, result.erosion);
		float terrainSelector = sampleNoise2(
			m_noiseGenerator, warpedX - 18000.0f, warpedZ + 12000.0f, TERRAIN_SELECTOR_NOISE);
		float plateauSelector = sampleNoise2(
			m_noiseGenerator, warpedX + 23000.0f, warpedZ + 19000.0f, PLATEAU_SELECTOR_NOISE);

		float flatWeight = glm::smoothstep(0.58f, 0.84f, result.erosion);
		float mountainWeight = inland * rugged * glm::smoothstep(0.58f, 0.80f, terrainSelector);
		float plateauWeight = inland * glm::smoothstep(0.60f, 0.80f, plateauSelector)
			* (1.0f - glm::smoothstep(0.72f, 0.90f, rugged));
		float highlandWeight = inland * rugged * glm::smoothstep(0.35f, 0.65f, terrainSelector)
			* (1.0f - mountainWeight);
		float rollingWeight = 0.35f + (1.0f - flatWeight) * 0.35f;

		result.terrainWeights = {
			flatWeight, rollingWeight, highlandWeight, plateauWeight, mountainWeight
		};
		float terrainWeightSum = 0.0f;
		for (float weight : result.terrainWeights) terrainWeightSum += weight;
		for (float& weight : result.terrainWeights) weight /= terrainWeightSum;

		float regionalMountainWeight = result.terrainWeights[static_cast<int>(TerrainType::Mountains)];
		result.mountainMask = regionalMountainWeight * glm::smoothstep(0.20f, 0.70f, result.ridge);
		result.coastMask = 1.0f - glm::smoothstep(
			0.0f, COAST_MASK_WIDTH, std::abs(result.continentalness - COAST_MASK_CENTER));

		float baseHeight;
		if (result.continentalness < OCEAN_CONTINENTALNESS)
		{
			float shelf = glm::smoothstep(0.05f, OCEAN_CONTINENTALNESS, result.continentalness);
			baseHeight = utils::lerp(DEEP_OCEAN_HEIGHT, COASTAL_SHELF_HEIGHT, shelf);
		}
		else if (result.continentalness < INLAND_CONTINENTALNESS)
		{
			float coast = glm::smoothstep(
				OCEAN_CONTINENTALNESS, INLAND_CONTINENTALNESS, result.continentalness);
			baseHeight = utils::lerp(COASTAL_SHELF_HEIGHT, COAST_HEIGHT, coast);
		}
		else
		{
			float inlandHeight = glm::smoothstep(
				INLAND_CONTINENTALNESS, 0.82f, result.continentalness);
			baseHeight = utils::lerp(COAST_HEIGHT, HIGH_INLAND_HEIGHT, inlandHeight);
		}

		float detail = sampleNoise2(
			m_noiseGenerator, warpedX + 7000.0f, warpedZ - 7000.0f, HEIGHT_DETAIL_NOISE) * 2.0f - 1.0f;
		float baseOffset = 0.0f;
		float hillStrength = 0.0f;
		for (size_t i = 0; i < result.terrainWeights.size(); ++i)
		{
			baseOffset += result.terrainWeights[i] * BASE_OFFSETS[i];
			hillStrength += result.terrainWeights[i] * HILL_STRENGTHS[i];
			result.densityStrength += result.terrainWeights[i] * DENSITY_STRENGTHS[i];
		}

		float plainsInfluence = m_plainsWeightSampler(result)
			* glm::smoothstep(0.48f, 0.78f, result.erosion);
		hillStrength *= utils::lerp(1.0f, 0.38f, plainsInfluence);
		result.densityStrength *= utils::lerp(1.0f, 0.55f, plainsInfluence);

		float formationRegion = sampleNoise2(
			m_noiseGenerator, warpedX + 41000.0f, warpedZ - 35000.0f, FORMATION_REGION_NOISE);
		float formationLand = glm::smoothstep(0.54f, 0.72f, result.continentalness);
		float formationRuggedness = 1.0f - glm::smoothstep(0.52f, 0.78f, result.erosion);
		result.formationMask = formationLand * formationRuggedness
			* glm::smoothstep(0.68f, 0.86f, formationRegion)
			* (1.0f - plainsInfluence);
		result.formationStrength = result.formationMask * FORMATION_MAX_STRENGTH;

		float plateauNoise = sampleNoise2(
			m_noiseGenerator, warpedX - 4000.0f, warpedZ + 4000.0f, PLATEAU_NOISE);
		float localPlateauCliff = 0.0f;
		float plateauSteps = terrace(
			plateauNoise, PLATEAU_STEP_COUNT, PLATEAU_TRANSITION_WIDTH, localPlateauCliff);
		float plateauWeightNormalized =
			result.terrainWeights[static_cast<int>(TerrainType::Plateaus)];
		result.plateauCliffMask = localPlateauCliff * plateauWeightNormalized
			* utils::lerp(1.0f, 0.25f, plainsInfluence);
		float plateauHeight = plateauSteps * PLATEAU_MAX_HEIGHT * plateauWeightNormalized
			* utils::lerp(1.0f, 0.30f, plainsInfluence);
		float plateauTopFlattening =
			1.0f - plateauWeightNormalized * (1.0f - result.plateauCliffMask) * 0.75f;
		float hills = detail * hillStrength * plateauTopFlattening;
		result.densityStrength += result.plateauCliffMask * 4.0f;
		float mountains = result.ridge * result.mountainMask * MOUNTAIN_MAX_HEIGHT
			* utils::lerp(1.0f, 0.20f, plainsInfluence);
		result.height = glm::clamp(
			baseHeight + baseOffset + hills + plateauHeight + mountains,
			static_cast<float>(MIN_WORLD_GEN_HEIGHT),
			static_cast<float>(MAX_WORLD_GEN_HEIGHT));
		return result;
	}

	float TerrainGenerator::sampleDensity(const TerrainSample& terrain, int x, int y, int z)
	{
		float baseDensity = terrain.height - static_cast<float>(y);
		float maximumDisplacement = terrain.densityStrength + terrain.formationStrength;
		if (baseDensity < -maximumDisplacement) return baseDensity;

		float density = baseDensity;
		if (baseDensity <= maximumDisplacement)
		{
			float densityNoise = m_noiseGenerator.noise3(
				x + 3107, y - 1973, z + 7919,
				DENSITY_NOISE.scale, DENSITY_NOISE.lacunarity,
				DENSITY_NOISE.persistence, DENSITY_NOISE.octaves) * 2.0f - 1.0f;
			density += densityNoise * terrain.densityStrength;
		}
		if (terrain.formationStrength > 0.01f)
		{
			float formationNoise = m_noiseGenerator.noise3(
				x - 15001.0f, y * FORMATION_VERTICAL_SCALE + 9000.0f, z + 19001.0f,
				FORMATION_NOISE.scale, FORMATION_NOISE.lacunarity,
				FORMATION_NOISE.persistence, FORMATION_NOISE.octaves) * 2.0f - 1.0f;
			density += formationNoise * terrain.formationStrength;
		}
		return density;
	}
}
