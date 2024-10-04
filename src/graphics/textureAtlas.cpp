#include "textureAtlas.hpp"

namespace voxel_game::graphics
{
	int texturesPerRow;
	std::unordered_map<std::string, glm::vec2> textureMap;

	void createTextureAtlas()
	{
		std::vector<std::filesystem::path> paths = voxel_game::utils::walkPath("res/texture/texture_atlas");

		texturesPerRow = static_cast<int>(std::ceil(std::sqrt(paths.size())));
		const int width = texturesPerRow * ATLAS_TEXTURE_SIZE;
		const int height = texturesPerRow * ATLAS_TEXTURE_SIZE;

		std::vector<unsigned char> result(width * height * 4, 255);

		stbi_set_flip_vertically_on_load(0);

		int x = 0, y = 0;
		for (std::filesystem::path path : paths)
		{
			int newWidth, newHeight, bitsPerPixel;
			unsigned char *image = stbi_load(path.string().c_str(), &newWidth, &newHeight, &bitsPerPixel, 4);

			for (int j = 0; j < newHeight; ++j)
			{
				for (int i = 0; i < newWidth; ++i)
				{
					int resultIndex = ((y + j) * width + (x + i)) * 4;
					int imageIndex = (j * newWidth + i) * 4;

					result[resultIndex] = image[imageIndex];
					result[resultIndex + 1] = image[imageIndex + 1];
					result[resultIndex + 2] = image[imageIndex + 2];
					result[resultIndex + 3] = image[imageIndex + 3];
				}
			}

			float texX = (float)x / width;
			float texY = 1 - (float)y / height - ATLAS_TEXTURE_SIZE / (float) height;
			textureMap[voxel_game::utils::getFileName(path)] = glm::vec2{texX, texY};

			log::info("Added " + path.string() + " to position: (" + std::to_string(texX) + "," + std::to_string(texY) + ")");

			x += ATLAS_TEXTURE_SIZE;
			if (x >= width)
			{
				x = 0;
				y += ATLAS_TEXTURE_SIZE;
			}

			stbi_image_free(image);
		}

		unsigned error = lodepng::encode(TEXTURE_ATLAS_PATH, result, width, height);
		if (error)
		{
			std::cout << "Error " << error << ": " << lodepng_error_text(error) << std::endl;
		}
	}

	int getTextureAtlasSize()
	{
		return ATLAS_TEXTURE_SIZE * texturesPerRow;
	}

	glm::vec2 getTextureAtlasCoords(AtlasTexture texture)
	{
		if (textureMap.find(getName(texture)) == textureMap.end())
		{
			// TODO: Adding error logging to logger
			std::cerr << "No texture found for " << (int) texture << std::endl; 
			return textureMap["placeholder"];
		}

		return textureMap[getName(texture)];
	}

	Texture* loadTextureAtlas()
	{
		return loadTexture(TEXTURE_ATLAS_PATH);
	}
}