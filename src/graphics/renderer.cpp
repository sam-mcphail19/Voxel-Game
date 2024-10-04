#include "renderer.hpp"

namespace voxel_game::graphics
{
	const glm::mat4 perspProjMat = glm::perspective(70.f, 16.f / 9, 0.1f, 1000.f);
	const glm::mat4 orthoProjMat = glm::ortho(-1, 1, -1, 1, -1, 1);

	Renderer::Renderer() {
		m_lastVertCountUpdateTime = std::chrono::system_clock::now();
	}

	void Renderer::render(Shader *shader, Camera *camera, Window *window)
	{
		clear();
		
		shader->setUniform1f(CURR_TIME_UNIFORM, utils::getElapsedTime());

		const glm::mat4 perspViewMat = camera->viewMatrix();

		shader->setUniformMat4(VIEW_UNIFORM, perspViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, perspProjMat);

		int totalVerts = 0;
		for (Mesh *mesh : m_perspMeshes)
		{
			shader->setUniformMat4(MODEL_UNIFORM, mesh->getTransform().modelMat());

			totalVerts += mesh->getVertexCount();
			mesh->render();
		}

		glClear(GL_DEPTH_BUFFER_BIT);

		const glm::mat4 orthViewMat = glm::mat4(1.f);

		shader->setUniformMat4(VIEW_UNIFORM, orthViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, orthoProjMat);

		for (Mesh *mesh : m_orthoMeshes)
		{
			shader->setUniformMat4(MODEL_UNIFORM, mesh->getTransform().modelMat());

			totalVerts += mesh->getVertexCount();
			mesh->render();
		}

		const auto now = std::chrono::system_clock::now();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastVertCountUpdateTime).count() > TIME_BETWEEN_VERT_COUNTER_UPDATES_MS)
		{
			log::info("Rendered " + std::to_string(totalVerts) + " total vertices");
			m_lastVertCountUpdateTime = now;
		}
	}

	void Renderer::submitPerspMesh(Mesh *mesh)
	{
		m_perspMeshes.push_back(mesh);
	}

	void Renderer::submitOrthoMesh(Mesh *mesh)
	{
		m_orthoMeshes.push_back(mesh);
	}

	void Renderer::clearMeshes()
	{
		m_perspMeshes.clear();
		m_orthoMeshes.clear();
	}

	void Renderer::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
