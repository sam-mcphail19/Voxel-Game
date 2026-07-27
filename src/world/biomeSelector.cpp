#include "biomeSelector.hpp"

#include <algorithm>
#include <glm/common.hpp>

namespace voxel_game::world
{
	namespace
	{
		constexpr float OCEAN_FULL_STRENGTH = 0.32f;
		constexpr float OCEAN_EDGE = 0.48f;

		constexpr float DESERT_INLAND_START = 0.50f;
		constexpr float DESERT_INLAND_END = 0.66f;
		constexpr float DESERT_HOT_START = 0.48f;
		constexpr float DESERT_HOT_END = 0.76f;
		constexpr float DESERT_DRY_START = 0.32f;
		constexpr float DESERT_DRY_END = 0.62f;
		constexpr float DESERT_TEMPERATURE_IMPORTANCE = 0.55f;
		constexpr float DESERT_HUMIDITY_IMPORTANCE = 0.45f;
		constexpr float DESERT_CLIMATE_START = 0.48f;
		constexpr float DESERT_CLIMATE_END = 0.78f;

		constexpr float MOUNTAIN_INLAND_START = 0.55f;
		constexpr float MOUNTAIN_INLAND_END = 0.80f;
		constexpr float MOUNTAIN_RUGGED_START = 0.35f;
		constexpr float MOUNTAIN_RUGGED_END = 0.75f;

		constexpr float PLAINS_LAND_START = 0.38f;
		constexpr float PLAINS_LAND_END = 0.52f;
		constexpr float PLAINS_GENTLE_START = 0.25f;
		constexpr float PLAINS_GENTLE_END = 0.70f;
		constexpr float PLAINS_BASE_WEIGHT = 0.35f;

		constexpr float CLEAR_BIOME_LEAD = 0.035f;
		constexpr float TRANSITION_NOISE_SCALE = 0.025f;
		constexpr int TRANSITION_NOISE_X_OFFSET = 34001;
		constexpr int TRANSITION_NOISE_Z_OFFSET = -27011;
	}

	BiomeSelector::BiomeSelector(long seed)
		: m_noiseGenerator(seed) {}

	std::vector<BiomeWeight> BiomeSelector::buildWeights(const TerrainSample& terrain) const
	{
		float ocean = 1.0f - glm::smoothstep(
			OCEAN_FULL_STRENGTH, OCEAN_EDGE, terrain.continentalness);

		float desertInland = glm::smoothstep(
			DESERT_INLAND_START, DESERT_INLAND_END, terrain.continentalness);
		float hot = glm::smoothstep(
			DESERT_HOT_START, DESERT_HOT_END, terrain.temperature);
		float dry = 1.0f - glm::smoothstep(
			DESERT_DRY_START, DESERT_DRY_END, terrain.humidity);
		float desertClimate = glm::smoothstep(
			DESERT_CLIMATE_START,
			DESERT_CLIMATE_END,
			hot * DESERT_TEMPERATURE_IMPORTANCE + dry * DESERT_HUMIDITY_IMPORTANCE);

		float mountainInland = glm::smoothstep(
			MOUNTAIN_INLAND_START, MOUNTAIN_INLAND_END, terrain.continentalness);
		float rugged = 1.0f - glm::smoothstep(
			MOUNTAIN_RUGGED_START, MOUNTAIN_RUGGED_END, terrain.erosion);

		float plainsLand = glm::smoothstep(
			PLAINS_LAND_START, PLAINS_LAND_END, terrain.continentalness);
		float gentle = glm::smoothstep(
			PLAINS_GENTLE_START, PLAINS_GENTLE_END, terrain.erosion);
		float plains = plainsLand
			* (PLAINS_BASE_WEIGHT + (1.0f - PLAINS_BASE_WEIGHT) * gentle);

		return {
			{BIOMES.at(BiomeType::Ocean), ocean},
			{BIOMES.at(BiomeType::Desert), desertInland * desertClimate},
			{BIOMES.at(BiomeType::Mountains), mountainInland * rugged},
			{BIOMES.at(BiomeType::Plains), plains}
		};
	}

	std::vector<BiomeWeight> BiomeSelector::normalize(std::vector<BiomeWeight> weights) const
	{
		float sum = 0.0f;
		for (auto& biomeWeight : weights)
		{
			biomeWeight.weight = glm::clamp(biomeWeight.weight, 0.0f, 1.0f);
			sum += biomeWeight.weight;
		}
		if (sum <= 0.0f)
		{
			for (auto& biomeWeight : weights)
			{
				biomeWeight.weight =
					biomeWeight.biome->type == BiomeType::Plains ? 1.0f : 0.0f;
			}
			return weights;
		}
		for (auto& biomeWeight : weights) biomeWeight.weight /= sum;
		return weights;
	}

	float BiomeSelector::getWeight(const TerrainSample& terrain, BiomeType biome) const
	{
		auto weights = normalize(buildWeights(terrain));
		auto match = std::find_if(weights.begin(), weights.end(), [biome](const BiomeWeight& weight) {
			return weight.biome->type == biome;
		});
		return match == weights.end() ? 0.0f : match->weight;
	}

	const Biome* BiomeSelector::select(std::vector<BiomeWeight> weights, int x, int z)
	{
		weights = normalize(std::move(weights));
		std::sort(weights.begin(), weights.end(), [](const BiomeWeight& a, const BiomeWeight& b) {
			return a.weight > b.weight;
		});

		const BiomeWeight& strongest = weights[0];
		const BiomeWeight& second = weights[1];
		if (strongest.weight - second.weight >= CLEAR_BIOME_LEAD) return strongest.biome;

		float strongestChance = strongest.weight / (strongest.weight + second.weight);
		float transitionNoise = m_noiseGenerator.noise2(
			x + TRANSITION_NOISE_X_OFFSET,
			z + TRANSITION_NOISE_Z_OFFSET,
			TRANSITION_NOISE_SCALE, 2.0f, 0.5f, 1);
		return transitionNoise < strongestChance ? strongest.biome : second.biome;
	}
}
