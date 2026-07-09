#include "renderer.hpp"

#include "../world/chunk.hpp"

namespace voxel_game::graphics
{
	static const glm::mat4 orthoProjMat = glm::ortho(-1, 1, -1, 1, -1, 1);
	static const glm::mat4 orthViewMat = glm::mat4(1.f);

	namespace
	{
		world::ChunkLod selectChunkLod(world::Chunk* chunk, Camera* camera)
		{
			world::BlockPos chunkCenter = chunk->getOrigin() + world::BlockPos{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };
			glm::vec3 cameraPos = camera->getPos();
			float x = cameraPos.x - chunkCenter.x;
			float z = cameraPos.z - chunkCenter.z;
			float distance = std::sqrt(x * x + z * z);

			if (distance >= CHUNK_LOD_2_DISTANCE_IN_BLOCKS)
			{
				return world::ChunkLod::QUARTER;
			}
			if (distance >= CHUNK_LOD_1_DISTANCE_IN_BLOCKS)
			{
				return world::ChunkLod::HALF;
			}
			return world::ChunkLod::FULL;
		}
	}

	Renderer::Renderer(const Window& window) : m_uiRenderer(UiRenderer(window)) {}

	void Renderer::renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera)
	{
		setupPerspRender(shader, camera);

		for (Mesh* mesh : meshes)
		{
			mesh->render();
		}
	}

	RenderStats Renderer::renderChunks(std::vector<world::Chunk*> chunks, Shader* shader, Camera* camera)
	{
		setupPerspRender(shader, camera);

		RenderStats stats;
		std::array<Plane, 6> frustum = buildCameraFrustum(camera);
		for (world::Chunk* chunk: chunks)
		{
			if (!chunkIntersectsFrustum(chunk, frustum))
			{
				stats.culledChunkCount++;
				continue;
			}

			std::unique_lock<std::mutex> lock = chunk->acquireLock();
			world::ChunkLod lod = selectChunkLod(chunk, camera);

			std::shared_ptr<g::Mesh> mesh = chunk->getMesh(lod);
			std::shared_ptr<g::Mesh> transparentMesh = chunk->getTransparentMesh(lod);

			if (mesh)
			{
				stats.vertexCount += mesh->getVertexCount();
				mesh->render();
			}
			if (transparentMesh)
			{
				stats.vertexCount += transparentMesh->getVertexCount();
				transparentMesh->render();
			}

			stats.renderedChunkCount++;
		}

		return stats;
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

	void Renderer::renderUi(bool isDebugEnabled, world::DebugInfo debugInfo)
	{
		if (isDebugEnabled)
		{
			m_uiRenderer.renderDebugInfo(debugInfo);
		}
	}

	void Renderer::setupPerspRender(Shader* shader, Camera* camera)
	{
		clear();

		shader->bind();

		shader->setUniform1f(CURR_TIME_UNIFORM, utils::getElapsedTime());

		const glm::mat4 perspViewMat = camera->viewMatrix();

		shader->setUniformMat4(VIEW_UNIFORM, perspViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, perspectiveProjectionMatrix());
	}

	void Renderer::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
