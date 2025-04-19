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
	private:
		VkImage colorImage;
		VkImageView colorImageView;
		VkDeviceMemory colorImageMemory;

		VkImage depthImage;
		VkImageView depthImageView;
		VkDeviceMemory depthImageMemory;

	public:
		void createColorResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples);
		void createImage(Engine::Graphics::Device device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t arrayLayers, VkImageCreateFlags flags);
		void createDepthResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples, Engine::Graphics::CommandBuffer commandBuffer);
		VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
		VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		void createFramebuffers(VkDevice device, Engine::Graphics::Swapchain& swapchain, VkRenderPass renderPass);
		void createBuffer(Engine::Graphics::Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		VkImage getColorImage() const { return colorImage; }
		VkImageView getColorImageView() const { return colorImageView; }
		VkDeviceMemory getColorImageMemory() const { return colorImageMemory; }

		VkImage getDepthImage() const { return depthImage; }
		VkImageView getDepthImageView() const { return depthImageView; }
		VkDeviceMemory getDepthImageMemory() const { return depthImageMemory; }
	};
}
#endif
