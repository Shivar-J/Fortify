#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <tiny_obj_loader.h>
#define TINYOBJLOADER_IMPLEMENTATION

#include "core.h"

int main() {
	Engine::Core::Application app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}