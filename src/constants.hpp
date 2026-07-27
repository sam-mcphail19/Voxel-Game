#pragma once

#include <string>

namespace voxel_game {

inline constexpr int MAX_KEYS = 1024;
inline constexpr int MAX_BUTTONS = 32;

inline constexpr int ATLAS_TEXTURE_SIZE = 16;
inline constexpr int ATLAS_PIXELS_PER_TEXTURE = ATLAS_TEXTURE_SIZE * ATLAS_TEXTURE_SIZE;

inline constexpr int CHUNK_SIZE = 16;
inline constexpr int CHUNK_HEIGHT = 256;
inline constexpr int CHUNK_SIZE_TIMES_HEIGHT = CHUNK_SIZE * CHUNK_HEIGHT;
inline constexpr int CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
inline constexpr int INITIAL_CHUNK_RENDER_DISTANCE = 8;
inline constexpr int CHUNK_GENERATION_BATCH_SIZE = 16;
inline constexpr int WORLD_HEIGHT = 512;
inline constexpr int WATER_HEIGHT = 72;
inline constexpr int MIN_WORLD_GEN_HEIGHT = 40;
inline constexpr int MAX_WORLD_GEN_HEIGHT = 300;
inline constexpr int CHUNK_RENDER_DISTANCE = 32;
inline constexpr int CHUNK_RENDER_DISTANCE_IN_BLOCKS = CHUNK_RENDER_DISTANCE * CHUNK_SIZE;
inline constexpr int CHUNK_LOD_LEVEL_COUNT = 3;
inline constexpr int CHUNK_LOD_1_DISTANCE_IN_BLOCKS = CHUNK_SIZE * 16;
inline constexpr int CHUNK_LOD_2_DISTANCE_IN_BLOCKS = CHUNK_SIZE * 24;

inline constexpr float GRAVITY = 0.01f;
inline constexpr float MOUSE_SENS = 0.24f;
inline constexpr float MOVEMENT_SPEED = 0.25f;
inline constexpr float JUMP_SPEED = 0.25f;
inline constexpr float CREATIVE_MOVEMENT_SPEED = 0.35f;

inline constexpr const char* MODEL_UNIFORM = "u_model";
inline constexpr const char* VIEW_UNIFORM  = "u_view";
inline constexpr const char* PROJ_UNIFORM  = "u_projection";
inline constexpr const char* CURR_TIME_UNIFORM = "u_currTime";
inline constexpr const char* IS_SELECTED_BLOCK_UNIFORM = "u_isSelectedBlock";
inline constexpr const char* SELECTED_BLOCK_UNIFORM = "u_selectedBlock";
inline constexpr const char* BLOCK_BREAK_PROG_UNIFORM = "u_blockBreakProgress";

inline constexpr const char* TEXTURE_ATLAS_PATH = "res/textureAtlas.png";

}
