#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "utility.h"

namespace Engine::Graphics {
	class Sampler;
	class Swapchain;
	class Device;

	class RenderPass
	{
	private:
		VkRenderPass renderPass;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSetLayout pbrDescriptorSetLayout;

	public:
		RenderPass() = default;
		~RenderPass();

	public:
		void createRenderPass(Engine::Graphics::Device device, VkSampleCountFlagBits msaaSamples, Engine::Graphics::Swapchain swapchain);
		VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
		void createDescriptorSetLayout(VkDevice device);
		void createDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
		void setupLayoutBindings(VkDevice device);

		VkRenderPass getRenderPass() const { return renderPass; }
		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
		VkDescriptorSetLayout getPBRDescriptorSetLayout() const { return pbrDescriptorSetLayout; }
	};
}
#endif