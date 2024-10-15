#pragma once

#include <chrono>
#include <iostream>
#include <vector>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "window.hpp"
#include "../world/chunk.hpp"
#include "../util/log.hpp"
#include "../util/timeUtils.hpp"

#define TIME_BETWEEN_VERT_COUNTER_UPDATES_MS 1000

namespace voxel_game::graphics
{
	class Renderer
	{
	public:
		Renderer();
		
		void renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera);
		void renderChunks(std::vector<world::Chunk*> chunks, Shader *shader, Camera *camera);
		void renderOrtho(std::vector<Mesh*> meshes, Shader *shader, Camera *camera);

	private:
		void setupPerspRender(Shader* shader, Camera* camera);
		void clear();
	};
}