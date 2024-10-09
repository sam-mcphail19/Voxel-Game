#pragma once

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/gtx/string_cast.hpp>
#include "texture.hpp"
#include "atlasTexture.hpp"
#include "../util/fileUtils.hpp"
#include "../util/log.hpp"
#include "../vendor/lodepng.h"
#include "../constants.hpp"

namespace voxel_game::graphics
{
	void createTextureAtlas();
	int getTextureAtlasSize();
	float getTextureAtlasTextureSize();
	Texture* loadTextureAtlas();
	glm::vec2 getTextureAtlasCoords(AtlasTexture texture);
}