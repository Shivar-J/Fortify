#ifndef RESOURCEMANAGER
#define RESOURCEMANAGER

#include <vulkan/vulkan.h>
#include "vulkanPointers.hpp"
#include <unordered_map>
#include <cstdint>

class Resource {
public:
	virtual void destroy(VkDevice device) = 0;
	virtual ~Resource() = default;
	virtual std::string log() = 0;
};

class BufferResource : public Resource {
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void* mapped = nullptr;

	VkResult initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* data = nullptr) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) return result;

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(device, buffer, &memReq);

		VkMemoryAllocateFlagsInfo memFlagsInfo{};
		memFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;

		if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
			memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties, physicalDevice);
		
		if (memFlagsInfo.flags != 0) {
			allocInfo.pNext = &memFlagsInfo;
		}
		else {
			allocInfo.pNext = nullptr;
		}

		result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		if (result != VK_SUCCESS) return result;

		result = vkBindBufferMemory(device, buffer, memory, 0);
		if (result != VK_SUCCESS) return result;

		if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			result = vkMapMemory(device, memory, 0, size, 0, &mapped);
			if (result != VK_SUCCESS) return result;
			/*
			if (data != nullptr) {
				memcpy(mapped, data, size);

				if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
					VkMappedMemoryRange range{};
					range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					range.memory = memory;
					range.offset = 0;
					range.size = size;
					vkFlushMappedMemoryRanges(device, 1, &range);
				}
			}*/
		}

		return VK_SUCCESS;
	}

	void destroy(VkDevice device) override {
		if (mapped) {
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
		if (buffer) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
		if (memory) {
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}

	std::string log() override {
		std::ostringstream ss;

		ss << "BufferResource | "
			<< "VkBuffer: " << buffer << ", "
			<< "VkDeviceMemory: " << memory << ", "
			<< "Mapped: " << (mapped ? "Yes" : "No") << "\n";

		return ss.str();
	}

private:
	static uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags prop, VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((type & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & prop) == prop) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type");
	}
};

class AccelerationStructureResource : public Resource {
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
	VkDeviceAddress address;

	VkResult initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = buildSizeInfo.accelerationStructureSize;
		bufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) return result;

		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice);
		result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);
		if (result != VK_SUCCESS) return result;

		result = vkBindBufferMemory(device, buffer, memory, 0);
		if (result != VK_SUCCESS) return result;

		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = buffer;
		accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = type;
		result = fpCreateAccelerationStructureKHR(device, &accelerationStructureCreateInfo, nullptr, &handle);
		if (result != VK_SUCCESS) return result;

		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = handle;
		address = fpGetAccelerationStructureDeviceAddressKHR(device, &accelerationDeviceAddressInfo);

		return VK_SUCCESS;
	}

	void destroy(VkDevice device) override {
		if (handle != VK_NULL_HANDLE) {
			fpDestroyAccelerationStructureKHR(device, handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
		if (buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
			address = 0;
		}
	}

	std::string log() override {
		std::ostringstream ss;

		ss << "AccelerationStructureResource | "
			<< "VkBuffer: " << buffer << ", "
			<< "VkDeviceMemory: " << memory << ", "
			<< "VkAccelerationStructureKHR: " << handle << ", "
			<< "VkDeviceAddress: " << address << "\n";

		return ss.str();
	}

private:
	static uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags prop, VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((type & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & prop) == prop) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type");
	}
};

class ImageResource : public Resource {
public:
	VkImage image = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkResult initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, bool isCube, bool useSampler) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.flags = flags;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = samples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) return result;

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device, image, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties, physicalDevice);

		result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		if (result != VK_SUCCESS) return result;

		vkBindImageMemory(device, image, memory, 0);

		if (aspectFlags != VK_IMAGE_ASPECT_NONE) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = mipLevels;
			viewInfo.subresourceRange.baseArrayLayer = 0;

			if (isCube) {
				viewInfo.subresourceRange.layerCount = 6;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			}
			else {
				viewInfo.subresourceRange.layerCount = 1;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			}

			result = vkCreateImageView(device, &viewInfo, nullptr, &view);
			if (result != VK_SUCCESS) return VK_SUCCESS;
		}

		if (useSampler) {
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;

			if (isCube) {
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.anisotropyEnable = VK_FALSE;
			}
			else {
				samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				samplerInfo.anisotropyEnable = VK_TRUE;
			}

			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(mipLevels);

			result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
			if (result != VK_SUCCESS) return result;
		}

		return VK_SUCCESS;
	}

	void destroy(VkDevice device) override {
		if (sampler) {
			vkDestroySampler(device, sampler, nullptr);
			sampler = VK_NULL_HANDLE;
		}
		if (view) {
			vkDestroyImageView(device, view, nullptr);
			view = VK_NULL_HANDLE;
		}
		if (image) {
			vkDestroyImage(device, image, nullptr);
			image = VK_NULL_HANDLE;
		}
		if (memory) {
			vkFreeMemory(device, memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}

	void updateLayout(VkImageLayout newLayout) {
		layout = newLayout;
	}

	std::string log() override {
		std::ostringstream ss;

		ss << "ImageResource | "
			<< "VkImage: " << image << ", "
			<< "VkDeviceMemory: " << memory << ", "
			<< "View: " << view << ", "
			<< "Sampler: " << sampler << ", "
			<< "Image Layout: " << layout << "\n";

		return ss.str();
	}

private:
	static uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags prop, VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((type & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & prop) == prop) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type");
	}
};

class SwapchainResource : public Resource {
public:
	VkSwapchainKHR swapchain;
	VkFormat format;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImageLayout> layouts;

	VkResult initialize(VkDevice device, VkSwapchainCreateInfoKHR swapchainInfo, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkExtent2D surfaceExtent) {
		VkResult result = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
		if (result != VK_SUCCESS) return result;

		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

		format = surfaceFormat.format;
		extent = surfaceExtent;

		views.resize(images.size());
		framebuffers.resize(images.size());
		layouts.resize(images.size());

		for (uint32_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = images[i];
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

			result = vkCreateImageView(device, &viewInfo, nullptr, &views[i]);

			layouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		return VK_SUCCESS;
	}

	void destroy(VkDevice device) override {
		for (auto fb : framebuffers) {
			vkDestroyFramebuffer(device, fb, nullptr);
		}
		for (auto view : views) {
			vkDestroyImageView(device, view, nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}

	std::string log() override {
		std::ostringstream ss;

		ss << "SwapchainResource | "
			<< "VkSwapchainKHR: " << swapchain << ", "
			<< "VkFormat: " << format << ", "
			<< "VkExtent: " << extent.width << "," << extent.height << "\n";

		for (int i = 0; i < images.size(); i++) {
			ss << "SwapchainResource | "
				<< "VkImage: " << images[i] << ", "
				<< "VkImageView: " << views[i] << ", "
				<< "VkFramebuffer: " << framebuffers[i] << ", "
				<< "VkImageLayout: " << layouts[i]  << "\n";
		}

		return ss.str();
	}

	void updateLayout(int index, VkImageLayout newLayout) {
		layouts[index] = newLayout;
	}
};

class ResourceManager
{
public:
	explicit ResourceManager(VkDevice device) : m_device(device) {};

	template<typename ResourceType, typename... Args>
	ResourceType* create(Args&&... args) {
		ResourceType* res = createResource<ResourceType>();
		if (res->initialize(std::forward<Args>(args)...) != VK_SUCCESS) {
			throw std::runtime_error("failed to initialize resource");
		}

		return res;
	}

	template<typename ResourceType, typename... Args>
	ResourceType* createResource(Args&&... args) {
		auto res = std::make_unique<ResourceType>(std::forward<Args>(args)...);
		ResourceType* ptr = res.get();
		m_resources.push_back(std::move(res));
		return ptr;
	}

	void destroy(Resource* resource) {
		const auto it = std::ranges::find_if(m_resources, [resource](const std::unique_ptr<Resource>& res) {
			return res.get() == resource;
			});

		if (it != m_resources.end()) {
			(*it)->destroy(m_device);
			m_resources.erase(it);
		}
	}

	void cleanup() {
		if (!m_resources.empty()) {
			for (const auto& resource : m_resources) {
				resource->destroy(m_device);
			}
			m_resources.clear();
		}
	}

    [[nodiscard]] std::string log() const {
		std::ostringstream ss;

		ss << "-------------Resource Log-------------\n";
		ss << "Total Resources: " << m_resources.size() << "\n";

		for (size_t i = 0; i < m_resources.size(); ++i) {
			ss << "Resource [" << i << "]: ";

			try {
				ss << m_resources[i]->log();
			}
			catch (...) {
				auto& res = *m_resources[i];
				ss << "Unknown Resource Type: " << typeid(res).name() << "\n";
			}
		}

		return ss.str();
    }

private:
	std::vector<std::unique_ptr<Resource>> m_resources;
	VkDevice m_device;
};

#endif

