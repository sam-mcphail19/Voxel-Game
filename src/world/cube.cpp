#include "cube.hpp"

namespace voxel_game::world
{
	Cube::Cube(std::vector<g::Quad*> faces)
	{
		m_faces = faces;
	}

	Cube::~Cube()
	{
		for (int i = 0; i < 6; i++) {
			delete m_faces[i];
		}
	}

	g::Quad* Cube::getFace(g::Direction direction)
	{
		return m_faces[(int) direction];
	}

	Cube singleTextureCube(world::Block block, g::AtlasTexture tex)
	{
		return multiTextureCube(block, tex, tex, tex, tex, tex, tex);
	}

	Cube multiTextureCube(
		world::Block block,
		g::AtlasTexture frontTex,
		g::AtlasTexture backTex,
		g::AtlasTexture leftTex,
		g::AtlasTexture rightTex,
		g::AtlasTexture topTex,
		g::AtlasTexture bottomTex)
	{
		std::vector<g::Quad*> faces = {
			g::Quad::createBlockQuad(block, g::Direction::FRONT, frontTex),
			g::Quad::createBlockQuad(block, g::Direction::BACK, backTex),
			g::Quad::createBlockQuad(block, g::Direction::RIGHT, rightTex),
			g::Quad::createBlockQuad(block, g::Direction::LEFT, leftTex),
			g::Quad::createBlockQuad(block, g::Direction::TOP, topTex),
			g::Quad::createBlockQuad(block, g::Direction::BOTTOM, bottomTex)
		};

		return Cube(faces);
	}
}