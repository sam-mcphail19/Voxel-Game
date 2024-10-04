cmake_minimum_required(VERSION 3.6)
project(VoxelGame)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

find_package(Threads REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(voxel_game 
	src/graphics/atlasTexture.cpp
	src/graphics/camera.cpp
	src/graphics/mesh.cpp
	src/graphics/quad.cpp
	src/graphics/renderer.cpp
	src/graphics/shader.cpp
	src/graphics/texture.cpp
	src/graphics/textureAtlas.cpp
	src/graphics/window.cpp
	src/input/input.cpp
	src/main.cpp
	src/physics/transform.cpp
	src/player.cpp
	src/util/fileUtils.cpp
	src/util/mathUtils.cpp
	src/util/stringUtils.cpp
	src/util/timeUtils.cpp
	src/vendor/lodepng.cpp
	src/vendor/OpenSimplex2S.cpp
	src/vendor/stb_image.cpp
	src/voxelGame.cpp
	src/world/block.cpp
	src/world/chunk.cpp
	src/world/chunkManager.cpp
	src/world/cube.cpp
	src/world/noiseGenerator.cpp
	src/world/world.cpp
	src/world/worldGenerator.cpp
)

target_link_libraries(voxel_game GLEW::glew)
target_link_libraries(voxel_game glfw)
target_link_libraries(voxel_game glm)
target_link_libraries(voxel_game OpenGL::GL)
target_link_libraries(voxel_game dbghelp)