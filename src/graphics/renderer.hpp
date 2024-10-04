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
	private:
		std::vector<Mesh *> m_perspMeshes;
		std::vector<Mesh *> m_orthoMeshes;
		std::chrono::time_point<std::chrono::system_clock> m_lastVertCountUpdateTime;
		void clear();

	public:
		Renderer();
		void render(Shader *shader, Camera *camera, Window *window);
		void submitPerspMesh(Mesh *mesh);
		void submitOrthoMesh(Mesh *mesh);
		void clearMeshes();
	};
}