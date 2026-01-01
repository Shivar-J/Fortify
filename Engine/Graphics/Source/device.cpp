#include "device.h"
#include "instance.h"
#include "swapchain.h"

Engine::Graphics::Device::~Device()
{
}

void Engine::Graphics::Device::pickPhysicalDevice(const Engine::Graphics::Instance& m_instance)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance.getInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance.getInstance(), &deviceCount, devices.data());

	for (const VkPhysicalDevice& m_device : devices) {
		if (isDeviceSuitable(m_device, m_instance.getSurface())) {
			this->physicalDevice = m_device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("failed to find a suitable GPU");
}

bool Engine::Graphics::Device::isDeviceSuitable(VkPhysicalDevice m_device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices = Engine::Graphics::Device::findQueueFamilies(m_device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(m_device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(m_device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(m_device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

Engine::Graphics::QueueFamilyIndices Engine::Graphics::Device::findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR m_surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;

	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
		
		if (presentSupport)
			indices.presentFamily = i;
		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

bool Engine::Graphics::Device::checkDeviceExtensionSupport(const VkPhysicalDevice m_device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(Engine::Settings::deviceExtensions.begin(), Engine::Settings::deviceExtensions.end());

	for (const auto&[extensionName, specVersion] : availableExtensions) {
		requiredExtensions.erase(extensionName);
	}

	return requiredExtensions.empty();
}

void Engine::Graphics::Device::createLogicalDevice(VkSurfaceKHR surface)
{
    Engine::Graphics::QueueFamilyIndices indices = Engine::Graphics::Device::findQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures vkFeatures{};
    vkFeatures.samplerAnisotropy = VK_TRUE;

    // Vulkan 1.2 promoted features
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures{};
    accelFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelFeatures.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures{};
    rtFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rtFeatures.rayTracingPipeline = VK_TRUE;
    rtFeatures.pNext = &accelFeatures;

    VkPhysicalDeviceVulkan12Features vk12Features{};
    vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Features.bufferDeviceAddress = VK_TRUE;
    vk12Features.descriptorIndexing = VK_TRUE;
    vk12Features.runtimeDescriptorArray = VK_TRUE;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vk12Features.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    vk12Features.pNext = &rtFeatures;

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.pNext = &vk12Features;
    deviceFeatures.features = vkFeatures;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(Engine::Settings::deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = Engine::Settings::deviceExtensions.data();

    if (Engine::Settings::enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(Engine::Settings::validationLayers.size());
        createInfo.ppEnabledLayerNames = Engine::Settings::validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    graphicsQueueFamilyIndex = indices.graphicsFamily.value();
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    presentQueueFamilyIndex = indices.presentFamily.value();
}

