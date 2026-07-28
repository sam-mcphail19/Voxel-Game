#pragma once

#include <string>
#include <functional>
#include "block.hpp"
#include "noiseGenerator.hpp"
#include "../util/mathUtils.hpp"
#include "../constants.hpp"

namespace voxel_game::world {
	enum class BiomeType {
		Plains,
		Mountains,
		Desert,
		Ocean,
		Marsh,
		Badlands,
		SnowyTundra,
		Taiga,
		Savanna,
		RockyHighlands,
		Rainforest,
		Volcanic,
		SaltFlats,
		Mesa,
		Glacier,
		StonyCoast,
	};

	struct Biome {
		BiomeType type;
		int (*getHeight)(int x, int z, NoiseGenerator& noiseGen);
		BlockTypeId (*blockFunc)(int x, int y, int z, int height);
	};

	extern const std::unordered_map<BiomeType, const Biome*> BIOMES;

	inline std::string toString(BiomeType biome)
	{
		switch (biome)
		{
		case BiomeType::Plains:
			return "Plains";
		case BiomeType::Mountains:
			return "Mountains";
		case BiomeType::Desert:
			return "Desert";
		case BiomeType::Ocean:
			return "Ocean";
		case BiomeType::Marsh:
			return "Marsh";
		case BiomeType::Badlands:
			return "Badlands";
		case BiomeType::SnowyTundra:
			return "Snowy Tundra";
		case BiomeType::Taiga:
			return "Taiga";
		case BiomeType::Savanna:
			return "Savanna";
		case BiomeType::RockyHighlands:
			return "Rocky Highlands";
		case BiomeType::Rainforest:
			return "Rainforest";
		case BiomeType::Volcanic:
			return "Volcanic";
		case BiomeType::SaltFlats:
			return "Salt Flats";
		case BiomeType::Mesa:
			return "Mesa";
		case BiomeType::Glacier:
			return "Glacier";
		case BiomeType::StonyCoast:
			return "Stony Coast";
		}

		return "Biome not found";
	}
};
