#pragma once
#include "instance.h"
#include "sampler.h"
#include "swapchain.h"

#include <vector>
#include <optional>
#include <set>
#include <string>

namespace Engine::Graphics {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class Device
	{
	public:
		inline static VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		inline static VkDevice device;
		inline static VkQueue graphicsQueue;
		inline static VkQueue presentQueue;

	public:
		static void pickPhysicalDevice();
		static bool isDeviceSuitable(VkPhysicalDevice device);
		static Engine::Graphics::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		static void createLogicalDevice();
	};
}


