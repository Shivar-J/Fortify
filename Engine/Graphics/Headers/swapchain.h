#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "utility.h"

namespace Engine::Core {
	class Application;
}

namespace Engine::Graphics {
	class Instance;
	class Device;
	class FrameBuffer;
	class ImageView;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			
			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}
	};

	class Swapchain
	{
	public:
		Swapchain() = default;
		~Swapchain();

	private:
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFrameBuffers;

	public:
		void createSwapChain(GLFWwindow* window, Engine::Graphics::Instance& instance, Engine::Graphics::Device& device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
		void cleanupSwapChain(Engine::Graphics::Device& device, Engine::Graphics::FrameBuffer& fb);
		//void recreateSwapChain();

		void createImageViews(VkDevice device);
		VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, bool isCube);

		VkSwapchainKHR getSwapchain() const { return swapChain; }
		std::vector<VkImage> getSwapchainImages() const { return swapChainImages; }
		VkFormat getSwapchainImageFormat() const { return swapChainImageFormat; }
		VkExtent2D getSwapchainExtent() const { return swapChainExtent; }
		std::vector<VkImageView> getSwapchainImageViews() const { return swapChainImageViews; }
		std::vector<VkFramebuffer> getSwapchainFramebuffers() const { return swapChainFrameBuffers; }
		std::vector<VkFramebuffer>& getSwapchainFramebuffers() { return swapChainFrameBuffers; }

		bool presentImmediate = false;
	};
};

#endif