#pragma once

#include <functional>

#include "biome.hpp"
#include "block.hpp"

namespace voxel_game::world
{
	class Chunk;

	struct SurfaceFeatureColumn
	{
		int surfaceHeight;
		BiomeType biome;
		BlockTypeId surfaceBlock;
		float slope;
		bool underwater;
	};

	class SurfaceFeatureGenerator
	{
	public:
		using ColumnSampler =
			std::function<SurfaceFeatureColumn(int worldX, int worldZ)>;

	private:
		long m_seed;

	public:
		explicit SurfaceFeatureGenerator(long seed);
		void generate(Chunk& chunk, const ColumnSampler& sampleColumn) const;
	};
}
