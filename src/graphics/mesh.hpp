#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "texture.hpp"
#include "../physics/transform.hpp"
#include "../util/log.hpp"
#include "../world/block.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#define POS_SIZE 3
#define NORMAL_SIZE 3
#define UV_SIZE 2
#define VERTEX_FLOAT_SIZE 8

#define FLAGS_SIZE 1
#define BLOCK_TYPE_SIZE 1
#define BLOCK_SIZE 3
#define VERTEX_INT_SIZE 5

namespace voxel_game::graphics
{
	struct Vertex
	{
		glm::vec3 m_position;
		glm::vec3 m_normal;
		glm::vec2 m_uv;
		int m_flags;
		int m_blockType;
		world::BlockPos m_blockPos;

		Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv, world::BlockTypeId blockTypeId, world::BlockPos blockPos, bool isBlockVert)
			: m_position(pos), m_normal(normal), m_uv(uv), m_blockType((int)blockTypeId), m_blockPos(blockPos)
		{
			m_flags = isBlockVert;
		}

		friend std::ostream &operator<<(std::ostream &os, const Vertex &vert)
		{
			std::stringstream flag;
			flag << "0x" << std::hex << vert.m_flags;
			std::stringstream blockType;
			blockType << "0x" << std::hex << vert.m_blockType;
			os << "Vertex(" << glm::to_string(vert.m_position) << ", " << glm::to_string(vert.m_normal) << ", " << glm::to_string(vert.m_uv) << ", " << flag.str();
			os << ", " << blockType.str() << ", " << vert.m_blockPos << ")";
			return os;
		}
	};

	class Mesh
	{
	private:
		std::vector<Vertex> m_vertices;
		std::vector<GLuint> m_indices;

		Texture *m_texture;
		physics::Transform m_transform;

		GLuint m_vao;
		GLuint m_ibo;
		GLuint m_fVbo;
		GLuint m_iVbo;

		bool m_isInit = false;

	private:
		void init();
		std::vector<float> createFloatBuffer();
		std::vector<int> createIntBuffer();

	public:
		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, physics::Transform transform, Texture *texture);
		~Mesh();
		void render();
		physics::Transform getTransform();
		std::vector<Vertex> getVertices();
		std::vector<GLuint> getIndices();
		int getVertexCount();
	};
}
