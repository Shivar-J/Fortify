#define GLM_FORCE_CXX17
#include <glm/glm.hpp>

#include <stdexcept>
#include <iostream>
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