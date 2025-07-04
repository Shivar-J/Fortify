#include "utility.h"

bool Engine::Settings::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayer(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayer.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayer) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}
	return true;
}

std::vector<const char*> Engine::Settings::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

uint32_t Engine::Utility::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

bool Engine::Utility::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;;
}

std::vector<const char*> Engine::Utility::getShaderPaths(const char*& path)
{
	std::vector<const char*> shaderPaths;

	shaderPaths.push_back("None");

	for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(path)) {
		if (dir_entry.path().extension() == ".spv") {
			shaderPaths.push_back(dir_entry.path().string().c_str());
		}
	}

	return shaderPaths;
}

VkTransformMatrixKHR Engine::Utility::convertMat4ToTransformMatrix(glm::mat4 m) {
	VkTransformMatrixKHR transform;
	for (uint32_t col = 0; col < 3; ++col) {
		for (uint32_t row = 0; row < 4; ++row) {
			transform.matrix[col][row] = m[row][col];
		}
	}
	return transform;
}

glm::mat4 Engine::Utility::convertTransformMatrixToMat4(VkTransformMatrixKHR mat)
{
	glm::mat4 res = {
		mat.matrix[0][0], mat.matrix[0][1], mat.matrix[0][2], mat.matrix[0][3],
		mat.matrix[1][0], mat.matrix[1][1], mat.matrix[1][2], mat.matrix[1][3],
		mat.matrix[2][0], mat.matrix[2][1], mat.matrix[2][2], mat.matrix[2][3],
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return res;
}
