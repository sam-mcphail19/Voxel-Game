#include "renderer.hpp"

namespace voxel_game::graphics
{
	static const glm::mat4 perspProjMat = glm::perspective(70.f, 16.f / 9, 0.1f, 1000.f);
	static const glm::mat4 orthoProjMat = glm::ortho(-1, 1, -1, 1, -1, 1);
	static const glm::mat4 orthViewMat = glm::mat4(1.f);

	Renderer::Renderer(const Window& window) : m_uiRenderer(UiRenderer(window)) {}

	void Renderer::renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera)
	{
		setupPerspRender(shader, camera);

		for (Mesh* mesh : meshes)
		{
			mesh->render();
		}
	}

	void Renderer::renderChunks(std::vector<world::Chunk*> chunks, Shader* shader, Camera* camera)
	{
		setupPerspRender(shader, camera);

		for (world::Chunk* chunk: chunks)
		{
			chunk->acquireLock();
			chunk->getMesh()->render();
			chunk->getTransparentMesh()->render();
		}
	}

	void Renderer::renderOrtho(std::vector<Mesh*> meshes, Shader* shader, Camera* camera)
	{
		glClear(GL_DEPTH_BUFFER_BIT); // Always render ortho meshes on top

		shader->bind();

		shader->setUniformMat4(VIEW_UNIFORM, orthViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, orthoProjMat);

		for (Mesh* mesh : meshes)
		{
			shader->setUniformMat4(MODEL_UNIFORM, mesh->getTransform()->modelMat());

			mesh->render();
		}
	}

	void Renderer::renderUi(bool isDebugEnabled)
	{
		if (isDebugEnabled)
		{
			m_uiRenderer.renderDebugInfo();
		}
	}

	void Renderer::setupPerspRender(Shader* shader, Camera* camera)
	{
		clear();

		shader->bind();

		shader->setUniform1f(CURR_TIME_UNIFORM, utils::getElapsedTime());

		const glm::mat4 perspViewMat = camera->viewMatrix();

		shader->setUniformMat4(VIEW_UNIFORM, perspViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, perspProjMat);
	}

	void Renderer::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
