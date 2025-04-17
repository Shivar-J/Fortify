#include "frameBuffer.h"
#include "commandBuffer.h"
#include "swapchain.h"
#include "renderPass.h"
#include "sampler.h"
#include "device.h"

void Engine::Graphics::FrameBuffer::createColorResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples)
{
	VkFormat colorFormat = swapchain.getSwapchainImageFormat();

	createImage(device, swapchain.getSwapchainExtent().width, swapchain.getSwapchainExtent().height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
	colorImageView = swapchain.createImageView(device.getDevice(), colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Engine::Graphics::FrameBuffer::createImage(Engine::Graphics::Device device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device.getDevice(), image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice());

	if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate image memory");

	vkBindImageMemory(device.getDevice(), image, imageMemory, 0);
}

void Engine::Graphics::FrameBuffer::createDepthResources(Engine::Graphics::Device device, Engine::Graphics::Swapchain swapchain, VkSampleCountFlagBits msaaSamples, Engine::Graphics::CommandBuffer commandBuffer)
{
	VkFormat depthFormat = findDepthFormat(device.getPhysicalDevice());
	createImage(device, swapchain.getSwapchainExtent().width, swapchain.getSwapchainExtent().height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = swapchain.createImageView(device.getDevice(), depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	commandBuffer.transitionImageLayout(device, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
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
			colorImageView,
			depthImageView,
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

void Engine::Graphics::FrameBuffer::createBuffer(Engine::Graphics::Device device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice());

	if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
}