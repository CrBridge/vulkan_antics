#include "va_terrain.hpp"

#include <stdexcept>
#include <iostream>

namespace va {
	std::shared_ptr<VaModel> VaTerrain::createTerrainFromFile(VaDevice& device, const std::string& filepath) {
		int width, height, channels;

		stbi_set_flip_vertically_on_load(false);
		std::string filepathAdj = FILE_DIR + filepath;
		stbi_uc* heightmapData = stbi_load(filepathAdj.c_str(), &width, &height, &channels, 0);
		if (!heightmapData) {
			throw std::runtime_error("failed to load texture image");
		}

		VaModel::Builder builder{};
		float yScale = 64.0f / 256.0f, yShift = 16.0f;

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				stbi_uc* texel = heightmapData + (j + width * i) * channels;
				stbi_uc y = texel[0];

				VaModel::Vertex vertex{};
				vertex.position = {
					(-height / 2.0f + height * i / (float)height),
					-1 * ((int)y * yScale - yShift),
					(-width / 2.0f + width * j / (float)width)
				};

				vertex.normal = { 0.0f, 1.0f, 0.0f };
				vertex.uv = { j / (float)width, i / (float)height };
				builder.vertices.push_back(vertex);
			}
		}

		for (unsigned int i = 0; i < height - 1; i++) {
			for (unsigned int j = 0; j < width - 1; j++) {
				builder.indices.push_back(j + width * i);
				builder.indices.push_back(j + width * (i + 1));
				builder.indices.push_back((j + 1) + width * i);

				builder.indices.push_back((j + 1) + width * i);
				builder.indices.push_back(j + width * (i + 1));
				builder.indices.push_back((j + 1) + width * (i + 1));
			}
		}
		stbi_image_free(heightmapData);

		return std::make_shared<VaModel>(device, builder);
	}
}