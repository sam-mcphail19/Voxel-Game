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
		Ocean
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
		}

		return "Biome not found";
	}
};
