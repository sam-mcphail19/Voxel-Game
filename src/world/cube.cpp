#include "cube.hpp"

namespace voxel_game::world
{
	Cube::Cube(std::map<g::Direction, g::Quad *> faces)
	{
		m_faces = faces;
	}

	std::map<g::Direction, g::Quad *> *Cube::getFaces()
	{
		return &m_faces;
	}

	Cube singleTextureCube(world::Block block, physics::Transform transform, g::AtlasTexture tex)
	{
		return multiTextureCube(block, transform, tex, tex, tex, tex, tex, tex);
	}

	Cube multiTextureCube(
		world::Block block, physics::Transform transform,
		g::AtlasTexture frontTex,
		g::AtlasTexture backTex,
		g::AtlasTexture leftTex,
		g::AtlasTexture rightTex,
		g::AtlasTexture topTex,
		g::AtlasTexture bottomTex)
	{
		std::map<g::Direction, g::Quad *> faces = {
			{g::Direction::FRONT, g::Quad::createBlockQuad(block, transform, g::Direction::FRONT, frontTex)},
			{g::Direction::BACK, g::Quad::createBlockQuad(block, transform, g::Direction::BACK, backTex)},
			{g::Direction::LEFT, g::Quad::createBlockQuad(block, transform, g::Direction::LEFT, leftTex)},
			{g::Direction::RIGHT, g::Quad::createBlockQuad(block, transform, g::Direction::RIGHT, rightTex)},
			{g::Direction::TOP, g::Quad::createBlockQuad(block, transform, g::Direction::TOP, topTex)},
			{g::Direction::BOTTOM, g::Quad::createBlockQuad(block, transform, g::Direction::BOTTOM, bottomTex)}};
		return Cube(faces);
	}
}