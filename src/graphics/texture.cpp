#include "texture.hpp"

namespace voxel_game::graphics
{
	std::unordered_map<std::string, Texture *> textureMap;

	Texture::Texture(GLuint id)
	{
		m_id = id;
	}

	void Texture::bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
	}

	void Texture::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
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
