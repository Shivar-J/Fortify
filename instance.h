#pragma once
#include "utility.h"
#include "core.h"
#include <iostream>

namespace Engine::Graphics {
	class Instance
	{
	public:
		inline static VkInstance instance;
		inline static VkDebugUtilsMessengerEXT debugMessenger;
		inline static VkSurfaceKHR surface;

	public:
		static void createInstance();

		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		static void setupDebugMessenger();
		static void createSurface();
	};
};


