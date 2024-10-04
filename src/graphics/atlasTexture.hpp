#pragma once

#include <map>
#include <string>

namespace voxel_game::graphics
{
	enum class AtlasTexture
	{
		STONE,
		DIRT,
		GRASS,
		GRASS_SIDE,
		BEDROCK
	};

	std::string getName(AtlasTexture tex);
}