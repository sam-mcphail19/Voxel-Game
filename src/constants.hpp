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

inline constexpr float GRAVITY = 0.01f;
inline constexpr float MOUSE_SENS = 0.24f;
inline constexpr float MOVEMENT_SPEED = 0.15f;
inline constexpr float JUMP_SPEED = 0.25f;
inline constexpr float CREATIVE_MOVEMENT_SPEED = 0.35f;

inline constexpr const char* MODEL_UNIFORM = "u_model";
inline constexpr const char* VIEW_UNIFORM  = "u_view";
inline constexpr const char* PROJ_UNIFORM  = "u_projection";
inline constexpr const char* CURR_TIME_UNIFORM = "u_currTime";
inline constexpr const char* IS_SELECTED_BLOCK_UNIFORM = "u_isSelectedBlock";
inline constexpr const char* SELECTED_BLOCK_UNIFORM = "u_selectedBlock";
inline constexpr const char* BLOCK_BREAK_PROG_UNIFORM = "u_blockBreakProgress";

extern int WORLD_HEIGHT;
extern int WATER_HEIGHT;
extern int MIN_WORLD_GEN_HEIGHT;
extern int MAX_WORLD_GEN_HEIGHT;
extern int CHUNK_RENDER_DISTANCE;
extern int CHUNK_RENDER_DISTANCE_IN_BLOCKS;

extern std::string TEXTURE_ATLAS_PATH;

}
