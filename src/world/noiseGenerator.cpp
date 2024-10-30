#include "noiseGenerator.hpp"

namespace voxel_game::world
{
	NoiseGenerator::NoiseGenerator(long seed) : m_simplex(new OpenSimplex2S(seed)) {}

    float NoiseGenerator::noise2(int x, int y, float scale, float lacunarity, float persistance, int octaves)
    {
        float sum = 0;
        float maxAmplitude = 0;

        for (int i = 0; i < octaves; i++)
        {
            float frequency = pow(lacunarity, i);
            float amplitude = pow(persistance, octaves - i);

            float noise = m_simplex->noise2(
                x * scale / frequency,
                y * scale / frequency
            );

            sum += noise * amplitude;
            maxAmplitude += amplitude;
        }

        return (sum / maxAmplitude + 1) / 2;
    }
}