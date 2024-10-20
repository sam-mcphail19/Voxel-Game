#include "worldGenerator.hpp"

namespace voxel_game::world
{
    WorldGenerator::WorldGenerator(long seed) : m_noiseGenerator(NoiseGenerator(seed))
    {
        m_heightMapMutex = new std::mutex();
    }

    void WorldGenerator::generateChunkData(Chunk &chunk)
    {
        const auto start = std::chrono::system_clock::now();
        BlockPos origin = chunk.getOrigin();
        for (int i = 0; i < CHUNK_BLOCK_COUNT; i++)
        {
            BlockPos pos = to3dIndex(i);
            chunk.putBlock(Block{pos, getBlockType(origin + pos)});
        }
        const auto end = std::chrono::system_clock::now();
        int durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        log::info("Generating chunk data took " + std::to_string(durationMs) + "ms");
    }

    BlockTypeId WorldGenerator::getBlockType(BlockPos pos)
    {
        if (pos.y < 4)
        {
            return BlockTypeId::BEDROCK;
        }

        int height = getHeight(pos.x, pos.z);

        if (pos.y > height)
        {
            if (pos.y > WATER_HEIGHT)
            {
                return BlockTypeId::AIR;
            }
            return BlockTypeId::WATER;
        }

        if (pos.y == height)
        {
            if (pos.y > WATER_HEIGHT)
            {
                return BlockTypeId::GRASS;
            }

            if (pos.y == WATER_HEIGHT)
            {
                return BlockTypeId::SAND;
            }

            return BlockTypeId::DIRT;
        }

        if (pos.y > height - 3)
        {
            return BlockTypeId::DIRT;
        }

        return BlockTypeId::STONE;
    }

    int WorldGenerator::getHeight(int x, int z)
    {
        int key = getHeightMapHash(x, z);
        std::unique_lock<std::mutex> lock(*m_heightMapMutex);

        if (m_heightMap.find(key) != m_heightMap.end())
        {
            return m_heightMap[key];
        }

        float noise = m_noiseGenerator.noise2(x, z, 0.05f, 2.f, 0.5f, 3);
        //float pv = fabs(m_noiseGenerator.noise2(x, z, 0.01f, 2.f, 0.5f, 1) * 2 - 1);

        int height = (int) (noise * WORLD_HEIGHT / 1.5f);
        m_heightMap[key] = height;
        return height;
    }

    int WorldGenerator::getHeightMapHash(int x, int z)
    {
        return CHUNK_BLOCK_COUNT * x + z;
    }
}