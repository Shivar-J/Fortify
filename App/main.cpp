#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <tiny_obj_loader.h>
#define TINYOBJLOADER_IMPLEMENTATION

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "core.h"
int main() {
	Engine::Core::Application app;

#ifdef _WIN32
	HWND hwnd = GetConsoleWindow();
	if (hwnd != nullptr) {
		ShowWindow(hwnd, SW_HIDE);
	}
#endif

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		system("pause");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}