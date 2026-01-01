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

		bool isComplete() const {
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
		uint32_t graphicsQueueFamilyIndex;
		uint32_t presentQueueFamilyIndex;

		VkPhysicalDeviceDescriptorIndexingFeaturesEXT enabledDescriptorIndexingFeatures{};
		VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddressFeatures{};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};
	public:
		Device() = default;
		~Device();

		//Device(const Device&) = delete;
		//Device& operator=(const Device&) = delete;

		[[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
		[[nodiscard]] VkDevice getDevice() const { return device; }
		[[nodiscard]] VkQueue getGraphicsQueue() const { return graphicsQueue; }
		[[nodiscard]] VkQueue getPresentQueue() const { return presentQueue; }
		[[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex; }
		[[nodiscard]] uint32_t getPresentQueueFamilyIndex() const { return presentQueueFamilyIndex; }

		void pickPhysicalDevice(const Engine::Graphics::Instance& m_instance);

		static bool isDeviceSuitable(VkPhysicalDevice m_device, VkSurfaceKHR surface);
		static Engine::Graphics::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR m_surface);

		static bool checkDeviceExtensionSupport(VkPhysicalDevice m_device);

		void createLogicalDevice(VkSurfaceKHR surface);
	};
}

#endif // !DEVICE_H