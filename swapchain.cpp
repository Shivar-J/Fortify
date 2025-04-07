#include "swapchain.h"

Engine::Graphics::SwapChainSupportDetails Engine::Graphics::Swapchain::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Engine::Graphics::Instance::surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Engine::Graphics::Instance::surface, &formatCount, nullptr);
	
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, Engine::Graphics::Instance::surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Engine::Graphics::Instance::surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, Engine::Graphics::Instance::surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void Engine::Graphics::Swapchain::createSwapChain()
{
	Engine::Graphics::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(Engine::Graphics::Device::physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = Engine::Graphics::Instance::surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	Engine::Graphics::QueueFamilyIndices indices = Engine::Graphics::Device::findQueueFamilies(Engine::Graphics::Device::physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(Engine::Graphics::Device::device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain");

	vkGetSwapchainImagesKHR(Engine::Graphics::Device::device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Engine::Graphics::Device::device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

VkSurfaceFormatKHR Engine::Graphics::Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}

VkPresentModeKHR Engine::Graphics::Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Engine::Graphics::Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else {
		int width, height;
		glfwGetFramebufferSize(Engine::Core::Application::window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void Engine::Graphics::Swapchain::cleanupSwapChain()
{
	vkDestroyImageView(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::depthImageView, nullptr);
	vkDestroyImage(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::depthImage, nullptr);
	vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::depthImageMemory, nullptr);

	vkDestroyImageView(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::colorImageView, nullptr);
	vkDestroyImage(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::colorImage, nullptr);
	vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::FrameBuffer::colorImageMemory, nullptr);

	for (auto framebuffer : swapChainFrameBuffers) {
		vkDestroyFramebuffer(Engine::Graphics::Device::device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(Engine::Graphics::Device::device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(Engine::Graphics::Device::device, swapChain, nullptr);
}

void Engine::Graphics::Swapchain::recreateSwapChain()
{
	int width = 0, height = 0;

	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(Engine::Core::Application::window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(Engine::Graphics::Device::device);

	cleanupSwapChain();

	createSwapChain();
	Engine::Graphics::ImageView::createImageViews();
	Engine::Graphics::FrameBuffer::createColorResources();
	Engine::Graphics::FrameBuffer::createDepthResources();
	Engine::Graphics::FrameBuffer::createFramebuffers();
}
