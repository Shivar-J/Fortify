#pragma once
#include "imageView.h"
#include "swapchain.h"
#include "renderPass.h"
#include <array>

namespace Engine::Graphics {
	class FrameBuffer
	{
	public:
		inline static VkImage colorImage;
		inline static VkImageView colorImageView;
		inline static VkDeviceMemory colorImageMemory;

		inline static VkImage depthImage;
		inline static VkImageView depthImageView;
		inline static VkDeviceMemory depthImageMemory;

	public:
		static void createColorResources();
		static void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		static void createDepthResources();
		static VkFormat findDepthFormat();
		static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		static void createFramebuffers();
		static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	};
}
