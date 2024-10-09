#pragma once

#include <chrono>
#include <iostream>
#include <vector>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "window.hpp"
#include "../util/log.hpp"
#include "../util/timeUtils.hpp"

#define TIME_BETWEEN_VERT_COUNTER_UPDATES_MS 1000

namespace voxel_game::graphics
{
	class Renderer
	{
	public:
		Renderer();
		void renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera, Window *window);
		void renderOrtho(std::vector<Mesh*> meshes, Shader *shader, Camera *camera, Window *window);
		void clear();
	};
}