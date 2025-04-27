#ifndef UTILITY_H
#define UTILITY_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_CXX17
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <cstdio>
#include <unordered_map>

#include "device.h"

namespace Engine::Settings {
	inline uint32_t WIDTH = 1280;
	inline uint32_t HEIGHT = 720;

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	inline uint32_t currentFrame = 0;

	inline const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	inline const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	inline const bool enableValidationLayers = false;
#else
	inline const bool enableValidationLayers = true;
#endif

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
};

namespace Engine::Utility {
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
	bool hasStencilComponent(VkFormat format);
}
#endif