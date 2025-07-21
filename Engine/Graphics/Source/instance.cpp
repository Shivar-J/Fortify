#include "instance.h"
#include "core.h"

Engine::Graphics::Instance::~Instance()
{

}

void Engine::Graphics::Instance::createInstance()
{
	if (Engine::Settings::enableValidationLayers && !Engine::Settings::checkValidationLayerSupport())
		throw std::runtime_error("Validation layers requested, not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Fortify";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Fortify Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = Engine::Settings::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (Engine::Settings::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Engine::Settings::validationLayers.size());
        createInfo.ppEnabledLayerNames = Engine::Settings::validationLayers.data();

        Engine::Graphics::Instance::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance");
    }
}

void Engine::Graphics::Instance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = Engine::Graphics::Instance::debugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Engine::Graphics::Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypeFlag, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    const char* msg = pCallbackData->pMessage;

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    g_logBuffer.push(std::string("[Validation] ") + msg + '\n', messageSeverity);

    return VK_FALSE;
}

VkResult Engine::Graphics::Instance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Engine::Graphics::Instance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

void Engine::Graphics::Instance::setupDebugMessenger()
{
    if (!Engine::Settings::enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Engine::Graphics::Instance::populateDebugMessengerCreateInfo(createInfo);

    if (Engine::Graphics::Instance::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to setup debug messenger");
}

void Engine::Graphics::Instance::createSurface(GLFWwindow* window)
{
    if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface");
}
