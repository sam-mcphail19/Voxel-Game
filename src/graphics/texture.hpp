#pragma once

#include <iostream>
#include <unordered_map>
#include <GL/glew.h>
#include "../vendor/stb_image.h"

namespace voxel_game::graphics
{
	class Texture
	{
	private:
		GLuint m_id;

	public:
		Texture(GLuint id);
		void bind();
		void unbind();
	};

	Texture* loadTexture(const char *resourcePath);
}