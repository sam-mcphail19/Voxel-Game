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
		GRAVEL,
		SANDSTONE,
		SANDSTONE_SIDE,
		MARSH_GRASS,
		MUD,
		RED_SANDSTONE,
		SNOW,
		ICE,
		PODZOL,
		DRY_GRASS,
		RAINFOREST_GRASS,
		BASALT,
		LAVA,
		SALT,
		TERRACOTTA,
		DECIDUOUS_LOG,
		DECIDUOUS_LEAVES,
		CONIFER_LOG,
		CONIFER_LEAVES,
		CACTUS,
	};

	const std::map<AtlasTexture, std::string> atlasTextureToNameMap = {
		{AtlasTexture::WATER, "water"},
		{AtlasTexture::STONE, "stone"},
		{AtlasTexture::DIRT, "dirt"},
		{AtlasTexture::GRASS, "grass"},
		{AtlasTexture::GRASS_SIDE, "grass_side"},
		{AtlasTexture::BEDROCK, "bedrock"},
		{AtlasTexture::SAND, "sand"},
		{AtlasTexture::GRAVEL, "gravel"},
		{AtlasTexture::SANDSTONE, "sandstone"},
		{AtlasTexture::SANDSTONE_SIDE, "sandstone_side"},
		{AtlasTexture::MARSH_GRASS, "marsh_grass"},
		{AtlasTexture::MUD, "mud"},
		{AtlasTexture::RED_SANDSTONE, "red_sandstone"},
		{AtlasTexture::SNOW, "snow"},
		{AtlasTexture::ICE, "ice"},
		{AtlasTexture::PODZOL, "podzol"},
		{AtlasTexture::DRY_GRASS, "dry_grass"},
		{AtlasTexture::RAINFOREST_GRASS, "rainforest_grass"},
		{AtlasTexture::BASALT, "basalt"},
		{AtlasTexture::LAVA, "lava"},
		{AtlasTexture::SALT, "salt"},
		{AtlasTexture::TERRACOTTA, "terracotta"},
		{AtlasTexture::DECIDUOUS_LOG, "deciduous_log"},
		{AtlasTexture::DECIDUOUS_LEAVES, "deciduous_leaves"},
		{AtlasTexture::CONIFER_LOG, "conifer_log"},
		{AtlasTexture::CONIFER_LEAVES, "conifer_leaves"},
		{AtlasTexture::CACTUS, "cactus"},
	};

	std::string getName(AtlasTexture tex);
}
