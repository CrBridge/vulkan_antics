#include "vk_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "models_meshes/va_terrain.hpp"

int main()
{
    va::VkApp app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}