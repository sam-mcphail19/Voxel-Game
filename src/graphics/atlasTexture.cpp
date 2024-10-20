#include "atlasTexture.hpp"

namespace voxel_game::graphics
{
	std::string getName(AtlasTexture tex)
	{
		return atlasTextureToNameMap.at(tex);
	}
}