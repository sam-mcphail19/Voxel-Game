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
		GLenum m_target;

	public:
		Texture(GLuint id, GLenum target = GL_TEXTURE_2D);
		void bind() const;
		void unbind();
		void upload(int width, int height, const unsigned char* pixels);
	};

	Texture* loadTexture(std::string resourcePath, bool forceReload = false);
	Texture* createTexture(int width, int height, const unsigned char* pixels);
	Texture* createTextureArray(
		int width,
		int height,
		int layerCount,
		const unsigned char* pixels);
}
