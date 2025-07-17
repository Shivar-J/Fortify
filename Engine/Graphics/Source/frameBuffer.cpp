#include "frameBuffer.h"
#include "commandBuffer.h"
#include "swapchain.h"
#include "renderPass.h"
#include "sampler.h"
#include "device.h"

void Engine::Graphics::FrameBuffer::createColorResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat = swapchain.getSwapchainImageFormat();

	colorResource = createImage(device.getDevice(), device.getPhysicalDevice(), swapchain.getSwapchainExtent().width, swapchain.getSwapchainExtent().height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_ASPECT_COLOR_BIT, false, false);
}

ImageResource* Engine::Graphics::FrameBuffer::createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, bool isCube, bool useSampler)
{
	return resources->create<ImageResource>(device, physicalDevice, width, height, mipLevels, samples, format, tiling, usage, properties, arrayLayers, flags, aspectFlags, isCube, useSampler);
}

void Engine::Graphics::FrameBuffer::createDepthResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples, Engine::Graphics::CommandBuffer commandBuffer)
{
	VkFormat depthFormat = findDepthFormat(device.getPhysicalDevice());
	depthResource = createImage(device.getDevice(), device.getPhysicalDevice(), swapchain.getSwapchainExtent().width, swapchain.getSwapchainExtent().height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_ASPECT_DEPTH_BIT, false, false);

	commandBuffer.transitionImageLayout(device, depthResource->image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
}

VkFormat Engine::Graphics::FrameBuffer::findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return Engine::Graphics::FrameBuffer::findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat Engine::Graphics::FrameBuffer::findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format");
}

void Engine::Graphics::FrameBuffer::createFramebuffers(VkDevice device, Engine::Graphics::Swapchain& swapchain, VkRenderPass renderPass)
{
	swapchain.getSwapchainFramebuffers().resize(swapchain.getSwapchainImageViews().size());

	for (size_t i = 0; i < swapchain.getSwapchainImageViews().size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorResource->view,
			depthResource->view,
			swapchain.getSwapchainImageViews()[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchain.getSwapchainExtent().width;
		framebufferInfo.height = swapchain.getSwapchainExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchain.getSwapchainFramebuffers()[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

BufferResource* Engine::Graphics::FrameBuffer::createBuffer(Engine::Graphics::Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* data)
{
	return resources->create<BufferResource>(device.getDevice(), device.getPhysicalDevice(), size, usage, properties, data);
}
