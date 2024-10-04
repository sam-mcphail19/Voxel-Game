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

#define TEXTURE_ATLAS_PATH "res/textureAtlas.png"
#define ATLAS_TEXTURE_SIZE 16
// 16 * 16
#define ATLAS_PIXELS_PER_TEXTURE 256

namespace voxel_game::graphics
{
	void createTextureAtlas();
	int getTextureAtlasSize();
	Texture* loadTextureAtlas();
	glm::vec2 getTextureAtlasCoords(AtlasTexture texture);
}