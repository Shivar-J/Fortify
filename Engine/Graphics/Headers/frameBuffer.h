#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "utility.h"

namespace Engine::Graphics {
	class CommandBuffer;
	class Swapchain;
	class RenderPass;
	class Sampler;
	class Device;

	class FrameBuffer
	{
	public:
		ImageResource* colorResource;
		ImageResource* depthResource;

	public:
		void createColorResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples);
		ImageResource* createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, bool isCube = false, bool useSampler = false);
		void createDepthResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples, Engine::Graphics::CommandBuffer commandBuffer);
		VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
		VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		void createFramebuffers(VkDevice device, Engine::Graphics::Swapchain& swapchain, VkRenderPass renderPass);
		BufferResource* createBuffer(Engine::Graphics::Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* data = nullptr);

	};
}
#endif
