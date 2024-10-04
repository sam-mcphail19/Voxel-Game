#include "atlasTexture.hpp"

namespace voxel_game::graphics
{
	std::map<AtlasTexture, std::string> atlasTextureToNameMap = {
		{AtlasTexture::STONE, "stone"},
		{AtlasTexture::DIRT, "dirt"},
		{AtlasTexture::GRASS, "grass"},
		{AtlasTexture::GRASS_SIDE, "grass_side"},
		{AtlasTexture::BEDROCK, "bedrock"}};

	std::string getName(AtlasTexture tex)
	{
		return atlasTextureToNameMap.at(tex);
	}
}