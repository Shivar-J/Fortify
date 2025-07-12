#ifndef RESOURCEMANAGER
#define RESOURCEMANAGER

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <cstdint>
#include <vma/vk_mem_alloc.h>

class Resource {
public:
	virtual void destroy(VmaAllocator allocator) = 0;
	virtual ~Resource() = default;
};

class BufferResource : public Resource {
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = nullptr;

	VkResult initialize(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memUsage;

		return initialize(allocator, bufferInfo, allocInfo);
	}

	VkResult initialize(VmaAllocator allocator, const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo) {
		return vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
	}
	
	void destroy(VmaAllocator allocator) override {
		if (buffer) {
			vmaDestroyBuffer(allocator, buffer, allocation);
			buffer = VK_NULL_HANDLE;
			allocation = nullptr;
		}
	}
};

class ResourceManager
{
public:
	explicit ResourceManager(VmaAllocator allocator) : m_allocator(allocator) {};

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
			(*it)->destroy(m_allocator);
			m_resources.erase(it);
		}
	}

	void cleanup() {
		for (auto& resource : m_resources) {
			resource->destroy(m_allocator);
		}
		m_resources.clear();
	}

private:
	std::vector<std::unique_ptr<Resource>> m_resources;
	VmaAllocator m_allocator;
};

#endif

