#pragma once

#include "../va_device.hpp"
#include "va_model.hpp"

#include <stb_image.h>

#include <string>

#ifndef FILE_DIR
#define FILE_DIR "../../../"
#endif

namespace va {
	class VaTerrain {
	public:
		static std::shared_ptr<VaModel> createTerrainFromFile(VaDevice& device, const std::string& filepath);

		VaTerrain() = default;
		VaTerrain(const VaTerrain&) = delete;
		VaTerrain& operator=(const VaTerrain&) = delete;
	};
}