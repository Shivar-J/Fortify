#pragma once
#include "swapchain.h"
#include "commandBuffer.h"

namespace Engine::Graphics {
	class ImageView
	{
	public:
		static void createImageViews();
		static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	};
}