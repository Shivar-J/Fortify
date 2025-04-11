#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include "utility.h"
#include "instance.h"
#include "device.h"

namespace Engine::Graphics {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Swapchain
	{
	public:
		inline static VkSwapchainKHR swapChain;
		inline static std::vector<VkImage> swapChainImages;
		inline static VkFormat swapChainImageFormat;
		inline static VkExtent2D swapChainExtent;
		inline static std::vector<VkImageView> swapChainImageViews;
		inline static std::vector<VkFramebuffer> swapChainFrameBuffers;

	public:
		static Engine::Graphics::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		static void createSwapChain();
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		static void cleanupSwapChain();
		static void recreateSwapChain();
	};
};

#endif