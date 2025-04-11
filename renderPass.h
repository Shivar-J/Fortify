#ifndef RENDERPASS_H
#define RENDERPASS_H
#include "swapchain.h"

namespace Engine::Graphics {
	class RenderPass
	{
	public:
		inline static VkRenderPass renderPass;
		inline static VkDescriptorSetLayout descriptorSetLayout;

	public:
		static void createRenderPass();
		static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		static VkFormat findDepthFormat();
		static void createDescriptorSetLayout();
	};
}
#endif