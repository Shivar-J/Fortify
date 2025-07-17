#ifndef RESOURCEMANAGER
#define RESOURCEMANAGER

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <cstdint>

class Resource {
public:
	virtual void destroy(VkDevice device) = 0;
	virtual ~Resource() = default;
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
		memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties, physicalDevice);
		
		result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
		if (result != VK_SUCCESS) return result;

		result = vkBindBufferMemory(device, buffer, memory, 0);
		if (result != VK_SUCCESS) return result;

		if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			result = vkMapMemory(device, memory, 0, size, 0, &mapped);
			if (result != VK_SUCCESS) return result;

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
			}
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

private:
	uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags prop, VkPhysicalDevice physicalDevice) {
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

private:
	uint32_t findMemoryType(uint32_t type, VkMemoryPropertyFlags prop, VkPhysicalDevice physicalDevice) {
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

	void destroy(Resource* resource, VkDevice device) {
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [resource](const std::unique_ptr<Resource>& res) {
			return res.get() == resource;
			});

		if (it != m_resources.end()) {
			(*it)->destroy(device);
			m_resources.erase(it);
		}
	}

	void cleanup() {
		if (!m_resources.empty()) {
			for (auto& resource : m_resources) {
				resource->destroy(m_device);
			}
			m_resources.clear();
		}
	}

	void log() {
		std::cout << "-------------Resource Log-------------" << std::endl;
		std::cout << "Total Resources: " << m_resources.size() << std::endl;

		for (size_t i = 0; i < m_resources.size(); i++) {
			const auto& res = m_resources[i];

			std::cout << "Resource [" << i << "]: ";

			if (auto buffer = dynamic_cast<BufferResource*>(res.get())) {
				std::cout << "BufferResource | "
					<< "VkBuffer: " << buffer->buffer << ", "
					<< "VkDeviceMemory: " << buffer->memory << ", "
					<< "Mapped: " << (buffer->mapped ? "Yes" : "No")
					<< std::endl;
			}
			else if (auto image = dynamic_cast<ImageResource*>(res.get())) {
				std::cout << "ImageResource | "
					<< "VkImage: " << image->image << ", "
					<< "VkDeviceMemory: " << image->memory << ", "
					<< "View: " << image->view << ", "
					<< "Sampler: " << image->sampler << std::endl;
			}
			else {
				std::cout << "Unknown Resource Type: " << typeid(*res).name() << std::endl;
			}
		}
	}

private:
	std::vector<std::unique_ptr<Resource>> m_resources;
	VkDevice m_device;
};

#endif

