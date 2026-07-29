#include "textureAtlas.hpp"

#include <algorithm>

namespace voxel_game::graphics
{
	int texturesPerRow;
	std::unordered_map<std::string, glm::vec2> textureAtlasMap;
	Texture* textureAtlas = nullptr;

	void createTextureAtlas()
	{
		std::vector<std::string> paths = voxel_game::utils::walkPath("res/texture/texture_atlas");
		std::sort(paths.begin(), paths.end());

		texturesPerRow = static_cast<int>(std::ceil(std::sqrt(paths.size())));
		const int width = texturesPerRow * ATLAS_TEXTURE_SIZE;
		const int height = texturesPerRow * ATLAS_TEXTURE_SIZE;

		std::vector<unsigned char> result(width * height * 4, 255);
		std::vector<unsigned char> layers(
			paths.size() * ATLAS_PIXELS_PER_TEXTURE * 4);

		stbi_set_flip_vertically_on_load(0);

		int x = 0, y = 0;
		for (size_t layer = 0; layer < paths.size(); ++layer)
		{
			const std::string& path = paths[layer];
			int newWidth, newHeight, bitsPerPixel;
			unsigned char *image = stbi_load(path.c_str(), &newWidth, &newHeight, &bitsPerPixel, 4);
			if (!image)
			{
				log::error("Could not load atlas texture " + path);
				continue;
			}
			if (newWidth != ATLAS_TEXTURE_SIZE || newHeight != ATLAS_TEXTURE_SIZE)
			{
				log::error(
					path + " must be " + std::to_string(ATLAS_TEXTURE_SIZE)
					+ "x" + std::to_string(ATLAS_TEXTURE_SIZE));
				stbi_image_free(image);
				continue;
			}

			for (int j = 0; j < newHeight; ++j)
			{
				for (int i = 0; i < newWidth; ++i)
				{
					int resultIndex = ((y + j) * width + (x + i)) * 4;
					int imageIndex = (j * newWidth + i) * 4;
					int flippedImageIndex =
						((newHeight - 1 - j) * newWidth + i) * 4;
					int layerIndex = (
						static_cast<int>(layer) * ATLAS_PIXELS_PER_TEXTURE
						+ j * newWidth + i) * 4;

					result[resultIndex] = image[imageIndex];
					result[resultIndex + 1] = image[imageIndex + 1];
					result[resultIndex + 2] = image[imageIndex + 2];
					result[resultIndex + 3] = image[imageIndex + 3];
					layers[layerIndex] = image[flippedImageIndex];
					layers[layerIndex + 1] = image[flippedImageIndex + 1];
					layers[layerIndex + 2] = image[flippedImageIndex + 2];
					layers[layerIndex + 3] = image[flippedImageIndex + 3];
				}
			}

			textureAtlasMap[voxel_game::utils::getFileName(path)] =
				glm::vec2{static_cast<float>(layer), 0.0f};

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
			log::error("Error " + std::to_string(error) + ": " + lodepng_error_text(error)); 
		}

		textureAtlas = createTextureArray(
			ATLAS_TEXTURE_SIZE,
			ATLAS_TEXTURE_SIZE,
			static_cast<int>(paths.size()),
			layers.data());
	}

	int getTextureAtlasSize()
	{
		return ATLAS_TEXTURE_SIZE * texturesPerRow;
	}

	float getTextureAtlasTextureSize()
	{
		return (float)ATLAS_TEXTURE_SIZE / getTextureAtlasSize();
	}

	glm::vec2 getTextureAtlasCoords(AtlasTexture texture)
	{
		const std::string& name = getName(texture);
		auto it = textureAtlasMap.find(name);

		if (it == textureAtlasMap.end())
		{
			log::error("No texture found for " + name);
			return textureAtlasMap["placeholder"];
		}

		return it->second;
	}

	glm::vec2 getTextureAtlasTileCoords(AtlasTexture texture)
	{
		return getTextureAtlasCoords(texture);
	}

	Texture* loadTextureAtlas()
	{
		return textureAtlas;
	}
}
