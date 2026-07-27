#include "biome.hpp"

namespace voxel_game::world
{
	namespace
	{
		constexpr int SOIL_DEPTH = 3;
	}

    static int plainsHeight(int x, int z, NoiseGenerator& n) {
        float height = n.noise2(x, z, 0.04f, 2.0f, 0.4f, 2);

		return WATER_HEIGHT + 4 + static_cast<int>(height * 8.0f);
    }

    static BlockTypeId plainsBlock(int x, int y, int z, int height) {
        if (y == height) return height < WATER_HEIGHT ? BlockTypeId::DIRT : BlockTypeId::GRASS;
        if (y > height - SOIL_DEPTH) return BlockTypeId::DIRT;
        return BlockTypeId::STONE;
    }

    static int mountainHeight(int x, int z, NoiseGenerator& n) {
        float base = n.noise2(x, z, 0.06f, 2.0f, 0.5f, 4);
		float ridgeNoise = n.noise2(x - 9999, z - 9999, 0.008f, 2.f, 0.4f, 5) * 2.0f - 1.0f;
		float ridged = 1.0f - glm::abs(ridgeNoise);
		ridged *= ridged;
        float h = ridged * (0.6f + 0.4f * base);

        float erosion = glm::clamp(1.0f - n.noise2(x + 5000, z + 5000, 0.1f, 2.f, 0.5f, 4), 0.3f, 1.0f);
        h *= erosion;

        h = glm::clamp(h, 0.0f, 1.0f);
        return static_cast<int>(utils::lerp(WATER_HEIGHT + 5, MAX_WORLD_GEN_HEIGHT, h));
    }

    // TODO: flatter areas should still be dirt, not stone
    static BlockTypeId mountainBlock(int x, int y, int z, int height) {
        if (y >= height)
        {
            return BlockTypeId::GRAVEL;
        }
            
        return BlockTypeId::STONE;
    }

    static int desertHeight(int x, int z, NoiseGenerator& n) {
        float noise = n.noise2(x, z, 0.05f, 2.0f, 0.5f, 2);
        return WATER_HEIGHT + 5 + static_cast<int>(noise * 5);
    }

    static BlockTypeId desertBlock(int x, int y, int z, int height) {
        if (y == height) return BlockTypeId::SAND;
        if (y > height - SOIL_DEPTH) return BlockTypeId::SANDSTONE;
        return BlockTypeId::STONE;
    }

    static int oceanHeight(int x, int z, NoiseGenerator& n) {
        float noise = n.noise2(x, z, 0.02f, 3.0f, 0.2f, 5);
        return WATER_HEIGHT - 10 - static_cast<int>(noise * 5);
    }

    static BlockTypeId oceanBlock(int x, int y, int z, int height) {
        if (y == height) return BlockTypeId::SAND;
        if (y > height - SOIL_DEPTH) return BlockTypeId::DIRT;
        return BlockTypeId::STONE;
    }

	static int marshHeight(int x, int z, NoiseGenerator& n) {
		float variation = n.noise2(x, z, 0.025f, 2.0f, 0.45f, 2);
		return WATER_HEIGHT + 1 + static_cast<int>(variation * 3.0f);
	}

	static BlockTypeId marshBlock(int x, int y, int z, int height) {
		if (y == height)
		{
			return height <= WATER_HEIGHT ? BlockTypeId::MUD : BlockTypeId::MARSH_GRASS;
		}
		if (y > height - SOIL_DEPTH) return BlockTypeId::MUD;
		return BlockTypeId::STONE;
	}

	static int badlandsHeight(int x, int z, NoiseGenerator& n) {
		float variation = n.noise2(x, z, 0.018f, 2.0f, 0.5f, 3);
		return WATER_HEIGHT + 12 + static_cast<int>(variation * 18.0f);
	}

	static BlockTypeId badlandsBlock(int x, int y, int z, int height) {
		if (y >= height - SOIL_DEPTH) return BlockTypeId::RED_SANDSTONE;
		return BlockTypeId::STONE;
	}

    static const Biome PLAINS = {
        BiomeType::Plains,
        plainsHeight,
        plainsBlock
    };
    
    static const Biome MOUNTAINS = {
        BiomeType::Mountains,
        mountainHeight,
        mountainBlock
    };
    
    static const Biome DESERT = {
        BiomeType::Desert,
        desertHeight,
        desertBlock
    };
    
    static const Biome OCEAN = {
        BiomeType::Ocean,
        oceanHeight,
        oceanBlock
    };

	static const Biome MARSH = {
		BiomeType::Marsh,
		marshHeight,
		marshBlock
	};

	static const Biome BADLANDS = {
		BiomeType::Badlands,
		badlandsHeight,
		badlandsBlock
	};

    const std::unordered_map<BiomeType, const Biome*> BIOMES = {
        { BiomeType::Plains,    &PLAINS },
        { BiomeType::Mountains, &MOUNTAINS },
        { BiomeType::Desert,    &DESERT },
        { BiomeType::Ocean,    &OCEAN },
		{ BiomeType::Marsh,     &MARSH },
		{ BiomeType::Badlands,  &BADLANDS },
    };
};
