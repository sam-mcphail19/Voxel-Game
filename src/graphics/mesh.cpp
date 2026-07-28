#include "mesh.hpp"

namespace voxel_game::graphics
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, physics::Transform* transform, Texture* texture)
		: m_vertices(std::move(vertices)),
		m_indices(std::move(indices)),
		m_vertexCount(m_vertices.size()),
		m_indexCount(m_indices.size()),
		m_texture(texture),
		m_transform(transform),
		m_isInit(false) {}

	Mesh::~Mesh()
	{
		if (m_isInit)
		{
			glDeleteBuffers(1, &m_fVbo);
			glDeleteBuffers(1, &m_iVbo);
			glDeleteBuffers(1, &m_ibo);
			glDeleteVertexArrays(1, &m_vao);
		}
	}

	void Mesh::render()
	{
		upload();

		m_texture->bind();

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);

		m_texture->unbind();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Mesh::upload()
	{
		if (!m_isInit)
		{
			init();
		}
	}

	bool Mesh::isUploaded() const
	{
		return m_isInit;
	}

	void Mesh::init()
	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenBuffers(1, &m_fVbo);
		glGenBuffers(1, &m_iVbo);
		glGenBuffers(1, &m_ibo);

		if (m_vertices.size() == 0)
		{
			m_isInit = true;
			return;
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glEnableVertexAttribArray(8);

		glBindBuffer(GL_ARRAY_BUFFER, m_fVbo);
		std::vector<float> floatBuffer = createFloatBuffer();
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * VERTEX_FLOAT_SIZE * sizeof(float), floatBuffer.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

		int floatBufferStride = VERTEX_FLOAT_SIZE * sizeof(float);
		// Position (x,y,z)
		glVertexAttribPointer(0, POS_SIZE, GL_FLOAT, GL_FALSE, floatBufferStride, 0);
		// Normal (x,y,z)
		glVertexAttribPointer(1, NORMAL_SIZE, GL_FLOAT, GL_FALSE, floatBufferStride, (void*)(POS_SIZE * sizeof(float)));
		// UVs (u,v)
		glVertexAttribPointer(2, UV_SIZE, GL_FLOAT, GL_FALSE, floatBufferStride, (void*)((POS_SIZE + NORMAL_SIZE) * sizeof(float)));
		// Atlas tile (x,y)
		glVertexAttribPointer(8, ATLAS_TILE_SIZE, GL_FLOAT, GL_FALSE, floatBufferStride, (void*)((POS_SIZE + NORMAL_SIZE + UV_SIZE) * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, m_iVbo);
		std::vector<int> intBuffer = createIntBuffer();
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * VERTEX_INT_SIZE * sizeof(int), &intBuffer[0], GL_STATIC_DRAW);

		int intBufferStride = VERTEX_INT_SIZE * sizeof(int);
		// Flags
		glVertexAttribIPointer(3, 1, GL_INT, intBufferStride, 0);
		// BlockType
		glVertexAttribIPointer(4, 1, GL_INT, intBufferStride, (void*)(1 * sizeof(int)));
		// Block (x,y,z)
		glVertexAttribIPointer(5, 1, GL_INT, intBufferStride, (void*)(2 * sizeof(int)));
		glVertexAttribIPointer(6, 1, GL_INT, intBufferStride, (void*)(3 * sizeof(int)));
		glVertexAttribIPointer(7, 1, GL_INT, intBufferStride, (void*)(4 * sizeof(int)));

		// The GPU buffers are now authoritative. Keeping the construction data
		// for every chunk and every LOD duplicates the largest mesh allocation.
		m_vertices.clear();
		m_vertices.shrink_to_fit();
		m_indices.clear();
		m_indices.shrink_to_fit();

		m_isInit = true;
	}

	std::vector<float> Mesh::createFloatBuffer()
	{
		std::vector<float> buffer(m_vertices.size() * VERTEX_FLOAT_SIZE);

		for (int i = 0; i < m_vertices.size(); i++)
		{
			buffer[i * VERTEX_FLOAT_SIZE] = m_vertices[i].m_position.x;
			buffer[i * VERTEX_FLOAT_SIZE + 1] = m_vertices[i].m_position.y;
			buffer[i * VERTEX_FLOAT_SIZE + 2] = m_vertices[i].m_position.z;
			buffer[i * VERTEX_FLOAT_SIZE + 3] = m_vertices[i].m_normal.x;
			buffer[i * VERTEX_FLOAT_SIZE + 4] = m_vertices[i].m_normal.y;
			buffer[i * VERTEX_FLOAT_SIZE + 5] = m_vertices[i].m_normal.z;
			buffer[i * VERTEX_FLOAT_SIZE + 6] = m_vertices[i].m_uv.x;
			buffer[i * VERTEX_FLOAT_SIZE + 7] = m_vertices[i].m_uv.y;
			buffer[i * VERTEX_FLOAT_SIZE + 8] = m_vertices[i].m_atlasTile.x;
			buffer[i * VERTEX_FLOAT_SIZE + 9] = m_vertices[i].m_atlasTile.y;
		}

		return buffer;
	}

	std::vector<int> Mesh::createIntBuffer()
	{
		std::vector<int> buffer(m_vertices.size() * VERTEX_INT_SIZE);

		for (int i = 0; i < m_vertices.size(); i++)
		{
			buffer[i * VERTEX_INT_SIZE] = m_vertices[i].m_flags;
			buffer[i * VERTEX_INT_SIZE + 1] = m_vertices[i].m_blockType;
			buffer[i * VERTEX_INT_SIZE + 2] = m_vertices[i].m_blockPos.x;
			buffer[i * VERTEX_INT_SIZE + 3] = m_vertices[i].m_blockPos.y;
			buffer[i * VERTEX_INT_SIZE + 4] = m_vertices[i].m_blockPos.z;
		}

		return buffer;
	}

	physics::Transform* Mesh::getTransform()
	{
		return m_transform;
	}

	std::vector<Vertex> Mesh::getVertices()
	{
		return m_vertices;
	}

	std::vector<GLuint> Mesh::getIndices()
	{
		return m_indices;
	}

	size_t Mesh::getVertexCount() const
	{
		return m_vertexCount;
	}

	size_t Mesh::getIndexCount() const
	{
		return m_indexCount;
	}

	size_t Mesh::getCpuStorageBytes() const
	{
		return m_vertices.capacity() * sizeof(Vertex) + m_indices.capacity() * sizeof(GLuint);
	}

	size_t Mesh::getEstimatedGpuBytes() const
	{
		const size_t vertexBytes = m_vertexCount
			* (VERTEX_FLOAT_SIZE * sizeof(float) + VERTEX_INT_SIZE * sizeof(int));
		return vertexBytes + m_indexCount * sizeof(GLuint);
	}
}
