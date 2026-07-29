#pragma once

#include <string>
#include <glm/vec3.hpp>

namespace voxel_game {

inline constexpr int MAX_KEYS = 1024;
inline constexpr int MAX_BUTTONS = 32;

inline constexpr int ATLAS_TEXTURE_SIZE = 64;
inline constexpr int ATLAS_PIXELS_PER_TEXTURE = ATLAS_TEXTURE_SIZE * ATLAS_TEXTURE_SIZE;

inline constexpr int CHUNK_SIZE = 16;
inline constexpr int CHUNK_HEIGHT = 256;
inline constexpr int CHUNK_SIZE_TIMES_HEIGHT = CHUNK_SIZE * CHUNK_HEIGHT;
inline constexpr int CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
inline constexpr int INITIAL_CHUNK_RENDER_DISTANCE = 8;
inline constexpr int CHUNK_GENERATION_BATCH_SIZE = 16;
inline constexpr int WORLD_HEIGHT = 768;
inline constexpr int WATER_HEIGHT = 72;
inline constexpr int MIN_WORLD_GEN_HEIGHT = 40;
inline constexpr int MAX_WORLD_GEN_HEIGHT = 680;
inline constexpr int CHUNK_RENDER_DISTANCE = 32;
inline constexpr int CHUNK_UNLOAD_DISTANCE = CHUNK_RENDER_DISTANCE + 4;
inline constexpr int CHUNK_RENDER_DISTANCE_IN_BLOCKS = CHUNK_RENDER_DISTANCE * CHUNK_SIZE;
inline constexpr int CHUNK_LOD_LEVEL_COUNT = 3;
inline constexpr int CHUNK_LOD_1_DISTANCE_IN_BLOCKS = CHUNK_SIZE * 16;
inline constexpr int CHUNK_LOD_2_DISTANCE_IN_BLOCKS = CHUNK_SIZE * 24;
// Full-resolution water surfaces use a uniform grid so displaced geometry
// and shoreline-distance interpolation share identical edges. Water walls
// may remain greedily merged because they do not carry ripple contours.
inline constexpr int WATER_SURFACE_MAX_QUAD_SPAN = 1;
inline constexpr int WATER_WALL_MAX_QUAD_SPAN = 1;
inline constexpr int WATER_SHORE_DISTANCE_BLOCKS = 4;

inline constexpr float GRAVITY = 0.01f;
inline constexpr float MOUSE_SENS = 0.24f;
inline constexpr float MOVEMENT_SPEED = 0.35f;
inline constexpr float JUMP_SPEED = 0.25f;
inline constexpr float CREATIVE_MOVEMENT_SPEED = 0.35f;
inline constexpr glm::vec3 SUN_DIRECTION = glm::vec3(-0.45f, 0.80f, 0.35f);
inline constexpr float AMBIENT_LIGHT_STRENGTH = 0.42f;
inline constexpr float SUN_LIGHT_STRENGTH = 0.72f;
inline constexpr float MIN_AMBIENT_OCCLUSION = 0.55f;
inline constexpr int SHADOW_MAP_RESOLUTION = 2048;
inline constexpr float SHADOW_MAP_RADIUS = 300.0f;
inline constexpr float SHADOW_FADE_START = 220.0f;
inline constexpr float SHADOW_LIGHT_DISTANCE = 500.0f;
inline constexpr float FOG_START_DISTANCE = 280.0f;
inline constexpr float FOG_END_DISTANCE = 500.0f;
inline constexpr float LOW_ALTITUDE_HAZE_STRENGTH = 0.18f;

inline constexpr const char* MODEL_UNIFORM = "u_model";
inline constexpr const char* VIEW_UNIFORM  = "u_view";
inline constexpr const char* PROJ_UNIFORM  = "u_projection";
inline constexpr const char* CURR_TIME_UNIFORM = "u_currTime";
inline constexpr const char* SUN_DIRECTION_UNIFORM = "u_sunDirection";
inline constexpr const char* AMBIENT_LIGHT_UNIFORM = "u_ambientLight";
inline constexpr const char* SUN_LIGHT_UNIFORM = "u_sunLight";
inline constexpr const char* MIN_AO_UNIFORM = "u_minAmbientOcclusion";
inline constexpr const char* LIGHT_SPACE_MATRIX_UNIFORM = "u_lightSpaceMatrix";
inline constexpr const char* SHADOW_MAP_UNIFORM = "u_shadowMap";
inline constexpr const char* CAMERA_POSITION_UNIFORM = "u_cameraPosition";
inline constexpr const char* FOG_START_UNIFORM = "u_fogStart";
inline constexpr const char* FOG_END_UNIFORM = "u_fogEnd";
inline constexpr const char* HAZE_STRENGTH_UNIFORM = "u_hazeStrength";
inline constexpr const char* FOG_BASE_HEIGHT_UNIFORM = "u_fogBaseHeight";
inline constexpr const char* ATLAS_TILE_SIZE_UNIFORM = "u_atlasTilePixelSize";
inline constexpr const char* IS_SELECTED_BLOCK_UNIFORM = "u_isSelectedBlock";
inline constexpr const char* SELECTED_BLOCK_UNIFORM = "u_selectedBlock";
inline constexpr const char* BLOCK_BREAK_PROG_UNIFORM = "u_blockBreakProgress";

inline constexpr const char* TEXTURE_ATLAS_PATH = "res/textureAtlas.png";

}
