#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <cstdint>

#include "device.h"

namespace Engine::Settings {
	inline uint32_t WIDTH = 1280;
	inline uint32_t HEIGHT = 720;

	inline const int MAX_FRAMES_IN_FLIGHT = 2;

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
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	bool hasStencilComponent(VkFormat format);
}