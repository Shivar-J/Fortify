#ifndef DEVICE_H
#define DEVICE_H

#include "utility.h"

namespace Engine::Graphics {
	class Instance;
	class Swapchain;
	class Sampler;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class Device
	{
	private:
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;

		VkPhysicalDeviceDescriptorIndexingFeaturesEXT enabledDescriptorIndexingFeatures{};
		VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddressFeatures{};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
	public:
		Device() = default;
		~Device();

		//Device(const Device&) = delete;
		//Device& operator=(const Device&) = delete;

		VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
		VkDevice getDevice() const { return device; }
		VkQueue getGraphicsQueue() const { return graphicsQueue; } 
		VkQueue getPresentQueue() const { return presentQueue; }

		void pickPhysicalDevice(const Engine::Graphics::Instance& instance);
		bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		Engine::Graphics::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		void createLogicalDevice(VkSurfaceKHR surface);
	};
}

#endif // !DEVICE_H