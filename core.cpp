#include "core.h"

void Engine::Core::Application::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void Engine::Core::Application::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	char title[256];
	title[255] = '\0';

	std::snprintf(title, 255, "%s %s - [FPS: %3.2f]", "Vulkan", "1.0.0", 0.0f);

	window = glfwCreateWindow(Engine::Settings::WIDTH, Engine::Settings::HEIGHT, title, nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Engine::Core::Application::initVulkan()
{
	Engine::Graphics::Instance::createInstance();
	Engine::Graphics::Instance::setupDebugMessenger();
	Engine::Graphics::Instance::createSurface();
	Engine::Graphics::Device::pickPhysicalDevice();
	Engine::Graphics::Device::createLogicalDevice();
	Engine::Graphics::Swapchain::createSwapChain();
	Engine::Graphics::ImageView::createImageViews();
	Engine::Graphics::RenderPass::createRenderPass();
	Engine::Graphics::RenderPass::createDescriptorSetLayout();
	Engine::Graphics::Pipeline::createGraphicsPipeline();
	Engine::Graphics::CommandBuffer::createCommandPool();
	Engine::Graphics::FrameBuffer::createColorResources();
	Engine::Graphics::FrameBuffer::createDepthResources();
	Engine::Graphics::FrameBuffer::createFramebuffers();
	Engine::Graphics::Texture::createTextureImage();
	Engine::Graphics::Texture::createTextureImageView();
	Engine::Graphics::Texture::createTextureSampler();
	Engine::Graphics::Texture::loadModel();
	Engine::Graphics::Texture::createVertexBuffer();
	Engine::Graphics::Texture::createIndexBuffer();
	Engine::Graphics::Texture::createUniformBuffers();
	Engine::Graphics::DescriptorSets::createDescriptorPool();
	Engine::Graphics::DescriptorSets::createDescriptorSets();
	Engine::Graphics::CommandBuffer::createCommandBuffers();
	Engine::Graphics::Texture::createSyncObjects();

}

void Engine::Core::Application::mainLoop()
{
	double prev = glfwGetTime();
	int frameCount = 0;

	while (!glfwWindowShouldClose(window)) {
		double curr = glfwGetTime();
		double deltaTime = curr - prev;
		frameCount++;

		if (deltaTime >= 1.0) {
			double fps = frameCount / deltaTime;

			char title[256];
			title[255] = '\0';

			std::snprintf(title, 255, "%s %s - [FPS: %3.2f]", "Fortify", "1.0.0", fps);

			glfwSetWindowTitle(window, title);
			prev = curr;
			frameCount = 0;
		}

		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(Engine::Graphics::Device::device);
}

void Engine::Core::Application::cleanup()
{
	Engine::Graphics::Swapchain::cleanupSwapChain();

	vkDestroyPipeline(Engine::Graphics::Device::device, Engine::Graphics::Pipeline::graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Engine::Graphics::Device::device, Engine::Graphics::Pipeline::pipelineLayout, nullptr);
	vkDestroyRenderPass(Engine::Graphics::Device::device, Engine::Graphics::RenderPass::renderPass, nullptr);

	for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(Engine::Graphics::Device::device, Engine::Graphics::Texture::uniformBuffers[i], nullptr);
		vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::Texture::uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(Engine::Graphics::Device::device, Engine::Graphics::DescriptorSets::descriptorPool, nullptr);

	vkDestroySampler(Engine::Graphics::Device::device, Engine::Graphics::Texture::textureSampler, nullptr);
	vkDestroyImageView(Engine::Graphics::Device::device, Engine::Graphics::Texture::textureImageView, nullptr);

	vkDestroyImage(Engine::Graphics::Device::device, Engine::Graphics::Texture::textureImage, nullptr);
	vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::Texture::textureImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(Engine::Graphics::Device::device, Engine::Graphics::RenderPass::descriptorSetLayout, nullptr);

	vkDestroyBuffer(Engine::Graphics::Device::device, Engine::Graphics::Texture::indexBuffer, nullptr);
	vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::Texture::indexBufferMemory, nullptr);

	vkDestroyBuffer(Engine::Graphics::Device::device, Engine::Graphics::Texture::vertexBuffer, nullptr);
	vkFreeMemory(Engine::Graphics::Device::device, Engine::Graphics::Texture::vertexBufferMemory, nullptr);

	for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(Engine::Graphics::Device::device, Engine::Graphics::Texture::renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(Engine::Graphics::Device::device, Engine::Graphics::Texture::imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Engine::Graphics::Device::device, Engine::Graphics::Texture::inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(Engine::Graphics::Device::device, Engine::Graphics::CommandBuffer::commandPool, nullptr);

	vkDestroyDevice(Engine::Graphics::Device::device, nullptr);

	if (Engine::Settings::enableValidationLayers) {
		Engine::Graphics::Instance::DestroyDebugUtilsMessengerEXT(Engine::Graphics::Instance::instance, Engine::Graphics::Instance::debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(Engine::Graphics::Instance::instance, Engine::Graphics::Instance::surface, nullptr);
	vkDestroyInstance(Engine::Graphics::Instance::instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void Engine::Core::Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Engine::Core::Application::drawFrame()
{
	vkWaitForFences(Engine::Graphics::Device::device, 1, &Engine::Graphics::Texture::inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(Engine::Graphics::Device::device, Engine::Graphics::Swapchain::swapChain, UINT64_MAX, Engine::Graphics::Texture::imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Engine::Graphics::Swapchain::recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swap chain image");

	Engine::Graphics::Texture::updateUniformBuffer(currentFrame);

	vkResetFences(Engine::Graphics::Device::device, 1, &Engine::Graphics::Texture::inFlightFences[currentFrame]);

	vkResetCommandBuffer(Engine::Graphics::CommandBuffer::commandBuffers[currentFrame], 0);
	Engine::Graphics::CommandBuffer::recordCommandBuffer(Engine::Graphics::CommandBuffer::commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { Engine::Graphics::Texture::imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &Engine::Graphics::CommandBuffer::commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { Engine::Graphics::Texture::renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(Engine::Graphics::Device::graphicsQueue, 1, &submitInfo, Engine::Graphics::Texture::inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { Engine::Graphics::Swapchain::swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(Engine::Graphics::Device::presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		Engine::Graphics::Swapchain::recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image");

	currentFrame = (currentFrame + 1) % Engine::Settings::MAX_FRAMES_IN_FLIGHT;
}