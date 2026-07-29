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

	Renderer::Renderer(const Window& window)
		: m_uiRenderer(UiRenderer(window)),
		m_windowWidth(window.getWidth()),
		m_windowHeight(window.getHeight())
	{
		glGenVertexArrays(1, &m_skyVao);

		glGenFramebuffers(1, &m_shadowFramebuffer);
		glGenTextures(1, &m_shadowDepthTexture);
		glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT24,
			SHADOW_MAP_RESOLUTION,
			SHADOW_MAP_RESOLUTION,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		const GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D,
			m_shadowDepthTexture,
			0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			log::error("Could not create the sun shadow framebuffer");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Renderer::~Renderer()
	{
		glDeleteVertexArrays(1, &m_skyVao);
		glDeleteTextures(1, &m_shadowDepthTexture);
		glDeleteFramebuffers(1, &m_shadowFramebuffer);
	}

	void Renderer::renderShadowMap(
		std::vector<world::Chunk*> chunks,
		Shader* shader,
		Camera* camera)
	{
		const glm::vec3 sunDirection = glm::normalize(SUN_DIRECTION);
		const glm::vec3 shadowCenter = camera->getPos();
		const glm::mat4 lightView = glm::lookAt(
			shadowCenter + sunDirection * SHADOW_LIGHT_DISTANCE,
			shadowCenter,
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightProjection = glm::ortho(
			-SHADOW_MAP_RADIUS,
			SHADOW_MAP_RADIUS,
			-SHADOW_MAP_RADIUS,
			SHADOW_MAP_RADIUS,
			1.0f,
			SHADOW_LIGHT_DISTANCE * 2.0f);

		// Keep the shadow texel grid fixed in world space. Without this snap,
		// following the camera moves the depth samples by fractions of a texel
		// and makes stationary shadows crawl across voxel faces.
		const glm::mat4 unsnappedLightSpace = lightProjection * lightView;
		glm::vec4 shadowOrigin =
			unsnappedLightSpace * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		shadowOrigin *= static_cast<float>(SHADOW_MAP_RESOLUTION) * 0.5f;
		const glm::vec2 roundedOrigin = glm::round(glm::vec2(shadowOrigin));
		glm::vec2 roundingOffset =
			roundedOrigin - glm::vec2(shadowOrigin);
		roundingOffset *= 2.0f
			/ static_cast<float>(SHADOW_MAP_RESOLUTION);
		lightProjection[3][0] += roundingOffset.x;
		lightProjection[3][1] += roundingOffset.y;
		m_lightSpaceMatrix = lightProjection * lightView;

		glViewport(
			0,
			0,
			SHADOW_MAP_RESOLUTION,
			SHADOW_MAP_RESOLUTION);
		glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.5f, 6.0f);

		shader->bind();
		shader->setUniformMat4(
			LIGHT_SPACE_MATRIX_UNIFORM,
			m_lightSpaceMatrix);
		const std::array<Plane, 6> lightFrustum =
			buildFrustum(m_lightSpaceMatrix);
		for (world::Chunk* chunk : chunks)
		{
			if (!chunkIntersectsFrustum(chunk, lightFrustum))
			{
				continue;
			}
			std::unique_lock<std::mutex> lock = chunk->acquireLock();
			std::shared_ptr<Mesh> mesh =
				chunk->getMesh(selectChunkLod(chunk, camera));
			if (mesh)
			{
				mesh->renderGeometry();
			}
		}

		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_windowWidth, m_windowHeight);
	}

	void Renderer::renderSky(Shader* shader, Camera* camera)
	{
		clear();

		shader->bind();
		shader->setUniformMat4(
			"u_inverseView",
			glm::inverse(camera->viewMatrix()));
		shader->setUniformMat4(
			"u_inverseProjection",
			glm::inverse(perspectiveProjectionMatrix()));
		shader->setUniform3f(
			SUN_DIRECTION_UNIFORM,
			glm::normalize(SUN_DIRECTION));

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(m_skyVao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	void Renderer::renderPersp(std::vector<Mesh*> meshes, Shader *shader, Camera *camera)
	{
		setupPerspRender(shader, camera);

		for (Mesh* mesh : meshes)
		{
			mesh->render();
		}
	}

	RenderStats Renderer::renderChunks(
		std::vector<world::Chunk*> chunks,
		Shader* terrainShader,
		Shader* waterShader,
		Camera* camera)
	{
		struct ChunkDrawMeshes
		{
			std::shared_ptr<g::Mesh> terrain;
			std::shared_ptr<g::Mesh> water;
		};

		RenderStats stats;
		std::vector<ChunkDrawMeshes> drawMeshes;
		drawMeshes.reserve(chunks.size());
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
			std::shared_ptr<g::Mesh> transparentMesh =
				chunk->getTransparentMesh(lod);

			if (mesh)
			{
				stats.vertexCount += mesh->getVertexCount();
			}
			if (transparentMesh)
			{
				stats.vertexCount += transparentMesh->getVertexCount();
			}

			drawMeshes.push_back({ mesh, transparentMesh });
			stats.renderedChunkCount++;
		}

		// Opaque terrain writes the complete depth buffer first. Water is then
		// blended back-to-front against that depth without hiding water behind
		// another transparent surface.
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		setupPerspRender(terrainShader, camera);
		for (const ChunkDrawMeshes& meshes : drawMeshes)
		{
			if (meshes.terrain)
			{
				meshes.terrain->render();
			}
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		setupPerspRender(waterShader, camera);
		for (const ChunkDrawMeshes& meshes : drawMeshes)
		{
			if (meshes.water)
			{
				meshes.water->render();
			}
		}
		glDepthMask(GL_TRUE);

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
		shader->bind();

		shader->setUniform1f(CURR_TIME_UNIFORM, utils::getElapsedTime());
		shader->setUniform3f(SUN_DIRECTION_UNIFORM, glm::normalize(SUN_DIRECTION));
		shader->setUniform1f(AMBIENT_LIGHT_UNIFORM, AMBIENT_LIGHT_STRENGTH);
		shader->setUniform1f(SUN_LIGHT_UNIFORM, SUN_LIGHT_STRENGTH);
		shader->setUniform1f(MIN_AO_UNIFORM, MIN_AMBIENT_OCCLUSION);
		shader->setUniformMat4(
			LIGHT_SPACE_MATRIX_UNIFORM,
			m_lightSpaceMatrix);
		shader->setUniform3f(
			CAMERA_POSITION_UNIFORM,
			camera->getPos());
		shader->setUniform1f(FOG_START_UNIFORM, FOG_START_DISTANCE);
		shader->setUniform1f(FOG_END_UNIFORM, FOG_END_DISTANCE);
		shader->setUniform1f(
			HAZE_STRENGTH_UNIFORM,
			LOW_ALTITUDE_HAZE_STRENGTH);
		shader->setUniform1f(
			FOG_BASE_HEIGHT_UNIFORM,
			static_cast<float>(WATER_HEIGHT));
		shader->setUniform1i(SHADOW_MAP_UNIFORM, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
		glActiveTexture(GL_TEXTURE0);

		const glm::mat4 perspViewMat = camera->viewMatrix();

		shader->setUniformMat4(VIEW_UNIFORM, perspViewMat);
		shader->setUniformMat4(PROJ_UNIFORM, perspectiveProjectionMatrix());
	}

	void Renderer::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
