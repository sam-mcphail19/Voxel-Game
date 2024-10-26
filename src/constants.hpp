#pragma once

#define MAX_KEYS 1024
#define MAX_BUTTONS 32

#define TEXTURE_ATLAS_PATH "res/textureAtlas.png"
#define ATLAS_TEXTURE_SIZE 16
constexpr int ATLAS_PIXELS_PER_TEXTURE = ATLAS_TEXTURE_SIZE * ATLAS_TEXTURE_SIZE;

#define WORLD_HEIGHT 256
#define WATER_HEIGHT 60
// TODO: This is only for NoiseTool, real mountains prob shouldn't work this way
// it should at least be varied
#define MOUNTAIN_HEIGHT 100
constexpr float GRAVITY = 0.01f;
#define CHUNK_RENDER_DISTANCE 2
#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 256
constexpr int CHUNK_SIZE_TIMES_HEIGHT = CHUNK_SIZE * CHUNK_HEIGHT;
constexpr int CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
#define CHUNK_RENDER_DISTANCE_IN_BLOCKS CHUNK_RENDER_DISTANCE * CHUNK_SIZE
#define MOUSE_SENS 0.24f
#define MOVEMENT_SPEED 0.15f
#define JUMP_SPEED 0.25f
#define CREATIVE_MOVEMENT_SPEED 0.35f

#define MODEL_UNIFORM "u_model"
#define VIEW_UNIFORM "u_view"
#define PROJ_UNIFORM "u_projection"
#define CURR_TIME_UNIFORM "u_currTime" 
#define IS_SELECTED_BLOCK_UNIFORM "u_isSelectedBlock"
#define SELECTED_BLOCK_UNIFORM "u_selectedBlock"
#define BLOCK_BREAK_PROG_UNIFORM "u_blockBreakProgress"