#pragma once

#include <map>
#include <string>

namespace voxel_game::graphics
{
	enum class AtlasTexture
	{
		WATER,
		STONE,
		DIRT,
		GRASS,
		GRASS_SIDE,
		BEDROCK,
		SAND,
	};

	const std::map<AtlasTexture, std::string> atlasTextureToNameMap = {
		{AtlasTexture::WATER, "water"},
		{AtlasTexture::STONE, "stone"},
		{AtlasTexture::DIRT, "dirt"},
		{AtlasTexture::GRASS, "grass"},
		{AtlasTexture::GRASS_SIDE, "grass_side"},
		{AtlasTexture::BEDROCK, "bedrock"},
		{AtlasTexture::SAND, "sand"},
	};

	std::string getName(AtlasTexture tex);
}