#include "constants.hpp"

namespace voxel_game {

int WORLD_HEIGHT = 512;
int WATER_HEIGHT = 72;
int MIN_WORLD_GEN_HEIGHT = 40;
int MAX_WORLD_GEN_HEIGHT = 300;
int CHUNK_RENDER_DISTANCE = 20;
int CHUNK_RENDER_DISTANCE_IN_BLOCKS = CHUNK_RENDER_DISTANCE * CHUNK_SIZE;

std::string TEXTURE_ATLAS_PATH = "res/textureAtlas.png";

}
