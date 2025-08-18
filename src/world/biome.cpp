#include "biome.hpp"

namespace voxel_game::world
{
    static int plainsHeight(int x, int z, NoiseGenerator& n) {
        float height = n.noise2(x, z, 0.04f, 2.0f, 0.4f, 2);

        return WATER_HEIGHT + static_cast<int>(height);
    }

    static BlockTypeId plainsBlock(int x, int y, int z, int height) {
        if (y == height) return BlockTypeId::GRASS;
        if (y > height - 3) return BlockTypeId::DIRT;
        return BlockTypeId::STONE;
    }

    static int mountainHeight(int x, int z, NoiseGenerator& n) {
        float base = n.noise2(x, z, 0.06f, 2.0f, 0.5f, 4);
        float ridged = glm::clamp(n.noise2(x - 9999, z - 9999, 0.08f, 2.f, 0.4f, 5), 0.f, 1.f);
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
        if (y > height - 3) return BlockTypeId::SANDSTONE;
        return BlockTypeId::STONE;
    }

    static int oceanHeight(int x, int z, NoiseGenerator& n) {
        float noise = n.noise2(x, z, 0.02f, 3.0f, 0.2f, 5);
        return WATER_HEIGHT - 10 - static_cast<int>(noise * 5);
    }

    static BlockTypeId oceanBlock(int x, int y, int z, int height) {
        if (glm::abs(y - WATER_HEIGHT) <= 4)
        {
            return BlockTypeId::SAND;
        }
        return BlockTypeId::DIRT;
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

    const std::unordered_map<BiomeType, const Biome*> BIOMES = {
        { BiomeType::Plains,    &PLAINS },
        { BiomeType::Mountains, &MOUNTAINS },
        { BiomeType::Desert,    &DESERT },
        { BiomeType::Ocean,    &OCEAN },
    };
};