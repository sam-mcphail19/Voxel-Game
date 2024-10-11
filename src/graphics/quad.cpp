#include "quad.hpp"

namespace voxel_game::graphics
{
	const int Quad::vertexPositions[] = {
		// Front Face
		0, 1, 1,
		0, 0, 1,
		1, 0, 1,
		1, 1, 1,

		// Back Face
		1, 0, 0,
		0, 0, 0,
		0, 1, 0,
		1, 1, 0,

		// Right Face
		1, 1, 1,
		1, 0, 1,
		1, 0, 0,
		1, 1, 0,

		// Left Face
		0, 1, 1,
		0, 1, 0,
		0, 0, 0,
		0, 0, 1,

		// Top Face
		0, 1, 0,
		0, 1, 1,
		1, 1, 1,
		1, 1, 0,

		// Bottom Face
		1, 0, 0,
		1, 0, 1,
		0, 0, 1,
		0, 0, 0,
	};

	const std::vector<GLuint> Quad::indices({ 0, 1, 3, 3, 1, 2 });

	const int Quad::uvs[] = {
		// Front Face
		0, 1,
		0, 0,
		1, 0,
		1, 1,

		// Back Face
		1, 0,
		0, 0,
		0, 1,
		1, 1,

		// Right Face
		0, 1,
		0, 0,
		1, 0,
		1, 1,

		// Left Face
		1, 1,
		0, 1,
		0, 0,
		1, 0,

		// Top Face
		0, 0,
		1, 0,
		1, 1,
		0, 1,

		// Bottom Face
		0, 0,
		0, 1,
		1, 1,
		1, 0,
	};

	const std::map<Direction, glm::vec3> Quad::normalMap = {
		{Direction::FRONT, glm::vec3(0, 0, 1)},
		{Direction::BACK, glm::vec3(0, 0, -1)},
		{Direction::RIGHT, glm::vec3(1, 0, 0)},
		{Direction::LEFT, glm::vec3(-1, 0, 0)},
		{Direction::TOP, glm::vec3(0, 1, 0)},
		{Direction::BOTTOM, glm::vec3(0, -1, 0)},
	};

	const std::map<Direction, int> Quad::vertexPositionIndexMap = {
		{Direction::FRONT, 0},
		{Direction::BACK, 12},
		{Direction::RIGHT, 24},
		{Direction::LEFT, 36},
		{Direction::TOP, 48},
		{Direction::BOTTOM, 60},
	};

	const std::map<Direction, int> Quad::uvIndexMap = {
		{Direction::FRONT, 0},
		{Direction::BACK, 8},
		{Direction::RIGHT, 16},
		{Direction::LEFT, 24},
		{Direction::TOP, 32},
		{Direction::BOTTOM, 40},
	};

	const int Quad::vertexCount = 6;

	Quad::Quad(std::vector<Vertex>* vertices, physics::Transform* transform, Texture* texture) :
		m_vertices(vertices), m_transform(transform), m_texture(texture) {}

	Quad::~Quad()
	{
		delete m_vertices;
	}

	Quad* Quad::createQuad(physics::Transform* transform, Texture* texture)
	{
		return new Quad(createVertices(), transform, texture);
	}

	Mesh* Quad::getMesh()
	{
		if (m_mesh)
		{
			return m_mesh;
		}

		m_mesh = new Mesh(*m_vertices, indices, m_transform, m_texture);
		return m_mesh;
	}

	std::vector<Vertex>* Quad::getVertices()
	{
		return m_vertices;
	}

	Quad* Quad::createBlockQuad(world::Block block, Direction direction, AtlasTexture texture)
	{
		return new Quad(createBlockQuadVertices(block, direction, texture), NULL, loadTextureAtlas());
	}

	glm::vec3 getNormal(Direction direction)
	{
		return Quad::normalMap.at(direction);
	}

	std::vector<Vertex>* Quad::createVertices()
	{
		std::vector<Vertex>* vertices = new std::vector<Vertex>;
		vertices->reserve(vertexCount);

		int startingIndex = vertexPositionIndexMap.at(Direction::FRONT);
		int uvIndex = uvIndexMap.at(Direction::FRONT);

		for (int i = 0; i < vertexCount; i++)
		{
			glm::vec3 pos = glm::vec3(
				vertexPositions[startingIndex + i * 3],
				vertexPositions[startingIndex + i * 3 + 1],
				vertexPositions[startingIndex + i * 3 + 2]);
			glm::vec2 uv = glm::vec2(uvs[uvIndex + i * 2], uvs[uvIndex + i * 2 + 1]);
			Vertex vert(pos, normalMap.at(Direction::FRONT), uv, world::BlockTypeId::NONE, { 0,0,0 }, false);
			vertices->push_back(vert);
		}

		return vertices;
	}

	std::vector<Vertex>* Quad::createBlockQuadVertices(world::Block block, Direction direction, AtlasTexture texture)
	{
		std::vector<Vertex>* vertices = new std::vector<Vertex>;
		vertices->reserve(vertexCount);

		int startingIndex = vertexPositionIndexMap.at(direction);
		int uvIndex = uvIndexMap.at(direction);
		glm::vec2 atlasCoords = getTextureAtlasCoords(texture);
		float textureSize = getTextureAtlasTextureSize();

		for (int i = 0; i < vertexCount; i++)
		{
			glm::vec3 pos = glm::vec3(
				vertexPositions[startingIndex + i * 3],
				vertexPositions[startingIndex + i * 3 + 1],
				vertexPositions[startingIndex + i * 3 + 2]
			);
			glm::vec2 uv = glm::vec2(
				uvs[uvIndex + i * 2] * textureSize + atlasCoords.x,
				uvs[uvIndex + i * 2 + 1] * textureSize + atlasCoords.y
			);
			Vertex vert(pos, normalMap.at(direction), uv, block.type, block.pos, true);
			vertices->push_back(vert);
		}

		return vertices;
	}
}