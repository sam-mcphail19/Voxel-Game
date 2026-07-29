#include "texture.hpp"

#include <algorithm>

namespace voxel_game::graphics
{
	std::unordered_map<std::string, Texture *> textureMap;

	namespace
	{
		void enableAnisotropicFiltering(GLenum target)
		{
			if (!GLEW_EXT_texture_filter_anisotropic)
			{
				return;
			}

			GLfloat maxAnisotropy = 1.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
			glTexParameterf(
				target,
				GL_TEXTURE_MAX_ANISOTROPY_EXT,
				std::min(8.0f, maxAnisotropy));
		}
	}

	Texture::Texture(GLuint id, GLenum target)
	{
		m_id = id;
		m_target = target;
	}

	void Texture::bind() const
	{
		glBindTexture(m_target, m_id);
	}

	void Texture::unbind()
	{
		glBindTexture(m_target, 0);
	}

	void Texture::upload(int width, int height, const unsigned char* pixels)
	{
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}

	Texture* createTexture(int width, int height, const unsigned char* pixels)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		Texture* texture = new Texture(id);
		texture->upload(width, height, pixels);
		return texture;
	}

	Texture* createTextureArray(
		int width,
		int height,
		int layerCount,
		const unsigned char* pixels)
	{
		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D_ARRAY, id);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(
			GL_TEXTURE_2D_ARRAY,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		// Preserve the crisp pixel-art appearance when a texel is magnified.
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			GL_RGBA8,
			width,
			height,
			layerCount,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels);
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		enableAnisotropicFiltering(GL_TEXTURE_2D_ARRAY);

		return new Texture(id, GL_TEXTURE_2D_ARRAY);
	}

	Texture* loadTexture(std::string resourcePath, bool forceReload)
	{
		if (!forceReload && textureMap.find(resourcePath) != textureMap.end())
		{
			return textureMap[resourcePath];
		}

		stbi_set_flip_vertically_on_load(1);
		int width, height, bitsPerPixel;
		unsigned char *buffer = stbi_load(resourcePath.c_str(), &width, &height, &bitsPerPixel, 4);

		Texture* texture = createTexture(width, height, buffer);

		if (buffer)
		{
			stbi_image_free(buffer);
		}

		textureMap[resourcePath] = texture;
		return textureMap[resourcePath];
	}
}
