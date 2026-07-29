#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "frustum.hpp"
#include "uiRenderer.hpp"
#include "window.hpp"
#include "../util/log.hpp"
#include "../util/timeUtils.hpp"

#define TIME_BETWEEN_VERT_COUNTER_UPDATES_MS 1000

namespace voxel_game::graphics
{
	namespace world = voxel_game::world;

	struct RenderStats
	{
		int vertexCount = 0;
		int renderedChunkCount = 0;
		int culledChunkCount = 0;
	};

	class Renderer
	{
	public:
		Renderer(const Window& window);
		~Renderer();
		
		void renderSky(Shader* shader, Camera* camera);
		void renderShadowMap(
			std::vector<world::Chunk*> chunks,
			Shader* shader,
			Camera* camera);
		void renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera);
		RenderStats renderChunks(
			std::vector<world::Chunk*> chunks,
			Shader* terrainShader,
			Shader* waterShader,
			Camera* camera);
		void renderOrtho(std::vector<Mesh*> meshes, Shader *shader, Camera *camera);
		void renderUi(bool isDebugEnabled, world::DebugInfo debugInfo);

	private:
		UiRenderer m_uiRenderer;
		GLuint m_skyVao = 0;
		GLuint m_shadowFramebuffer = 0;
		GLuint m_shadowDepthTexture = 0;
		int m_windowWidth;
		int m_windowHeight;
		glm::mat4 m_lightSpaceMatrix = glm::mat4(1.0f);

		void setupPerspRender(Shader* shader, Camera* camera);
		void clear();
	};
}
