#ifndef INSTANCE_H
#define INSTANCE_H

#include "utility.h"

namespace Engine::Core {
	class Application;
}

namespace Engine::Graphics {
	class Instance
	{
	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;

	public:
		Instance() = default;
		~Instance();

		//disables copy constructor and copy assignment operator (*)
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		void createInstance();
		void setupDebugMessenger();
		void createSurface(GLFWwindow* window);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		VkInstance getInstance() const { return m_instance; }
		VkSurfaceKHR getSurface() const { return m_surface; }
		VkDebugUtilsMessengerEXT getDebugMessenger() const { return m_debugMessenger; }

	private:
		static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	};
};

#endif
