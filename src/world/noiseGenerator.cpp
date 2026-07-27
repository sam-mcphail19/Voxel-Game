#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	NoiseGenerator::NoiseGenerator(long seed) : m_simplex(seed) {}

    float NoiseGenerator::noise2(int x, int y, float scale, float lacunarity, float persistance, int octaves)
    {
		return noise2(static_cast<float>(x), static_cast<float>(y), scale, lacunarity, persistance, octaves);
	}

	float NoiseGenerator::noise2(float x, float y, float scale, float lacunarity, float persistence, int octaves)
	{
        float sum = 0;
        float maxAmplitude = 0;
		float frequency = 1.0f;
		float amplitude = 1.0f;

        for (int i = 0; i < octaves; i++)
        {
			float noise = static_cast<float>(m_simplex.noise2(
                x * scale * frequency,
                y * scale * frequency
			));

            sum += noise * amplitude;
            maxAmplitude += amplitude;
			frequency *= lacunarity;
			amplitude *= persistence;
        }

        return (sum / maxAmplitude + 1) / 2;
    }

    float NoiseGenerator::noise3(int x, int y, int z, float scale, float lacunarity, float persistance, int octaves)
    {
		return noise3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), scale, lacunarity, persistance, octaves);
	}

	float NoiseGenerator::noise3(float x, float y, float z, float scale, float lacunarity, float persistence, int octaves)
	{
        float sum = 0;
        float maxAmplitude = 0;
		float frequency = 1.0f;
		float amplitude = 1.0f;

        for (int i = 0; i < octaves; i++)
        {
			float noise = static_cast<float>(m_simplex.noise3_XZBeforeY(
                x * scale * frequency,
                y * scale * frequency,
                z * scale * frequency
			));

            sum += noise * amplitude;
            maxAmplitude += amplitude;
			frequency *= lacunarity;
			amplitude *= persistence;
        }

        return (sum / maxAmplitude + 1) / 2;
    }
}
