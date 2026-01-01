#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <tiny_obj_loader.h>
#define TINYOBJLOADER_IMPLEMENTATION

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "core.h"
#include <cstdlib>

int main() {
#ifdef _WIN32
	HWND hwnd = GetConsoleWindow();
	if (hwnd != nullptr) {
		ShowWindow(hwnd, SW_HIDE);
	}
#endif

	try {
		Engine::Core::Application app;
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
#ifdef _WIN32
		system("pause");
#else
		std::cerr << "Error: " << e.what() << std::endl;
#endif
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}