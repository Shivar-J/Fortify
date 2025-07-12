#ifndef UTILITY_H
#define UTILITY_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_CXX17
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>
#include <cstdio>
#include <unordered_map>
#include <filesystem>
#include <numeric>

#include "device.h"
#include "ResourceManager.h"

extern VmaAllocator allocator;
extern std::unique_ptr<ResourceManager> resources;

struct Vertex {
	alignas (16) glm::vec3 pos;
	alignas (16) glm::vec3 normal;
	alignas (16) glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription{};

		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(Vertex, pos);

		attributeDescription[1].binding = 0;
		attributeDescription[1].location = 1;
		attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[1].offset = offsetof(Vertex, normal);

		attributeDescription[2].binding = 0;
		attributeDescription[2].location = 2;
		attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[2].offset = offsetof(Vertex, texCoord);

		return attributeDescription;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct CubeVertex {
	glm::vec3 pos;

	bool operator==(const CubeVertex& other) const {
		return pos == other.pos;
	}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(CubeVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescription() {
		std::array<VkVertexInputAttributeDescription, 1> attributeDescription{};

		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[0].offset = offsetof(CubeVertex, pos);

		return attributeDescription;
	}
};

struct Materials {
	std::string name;
	std::string diffusePath;
	std::string normalPath;
	std::string roughnessPath;
	std::string metalnessPath;
	std::string aoPath;
	std::string specularPath;

	VkImage diffuseImage;
	VkImageView diffuseImageView;
	VkSampler diffuseSampler;
	VkDeviceMemory diffuseImageMemory;

	VkImage normalImage;
	VkImageView normalImageView;
	VkSampler normalSampler;
	VkDeviceMemory normalImageMemory;

	VkImage roughnessImage;
	VkImageView roughnessImageView;
	VkSampler roughnessSampler;
	VkDeviceMemory roughnessImageMemory;

	VkImage metalnessImage;
	VkImageView metalnessImageView;
	VkSampler metalnessSampler;
	VkDeviceMemory metalnessImageMemory;

	VkImage aoImage;
	VkImageView aoImageView;
	VkSampler aoSampler;
	VkDeviceMemory aoImageMemory;

	VkImage specularImage;
	VkImageView specularImageView;
	VkSampler specularSampler;
	VkDeviceMemory specularImageMemory;
};

struct MeshObject {
	std::vector<Vertex> v;
	std::vector<uint32_t> i;
	std::vector<Materials> m;

	VkBuffer vb = VK_NULL_HANDLE;
	VkDeviceMemory vbm = VK_NULL_HANDLE;
	VkBuffer ib = VK_NULL_HANDLE;
	VkDeviceMemory ibm = VK_NULL_HANDLE;
	VkBuffer mb = VK_NULL_HANDLE;
	VkDeviceMemory mbm = VK_NULL_HANDLE;

	std::string path = "";

	void destroy(VkDevice device);
};

enum class PBRTextureType {
	Albedo = 1,
	Normal = 2,
	Roughness = 3,
	Metalness = 4,
	AmbientOcclusion = 5,
	Specular = 6,
};

enum class EntityType {
	Object,
	Skybox,
	UI,
	Light,
	Terrain,
	Particle,
	PBRObject,
	MatObject,
	Primitive
};

enum class ShaderType {
	Vertex,
	Fragment,
	rgen,
	rchit,
	rmiss
};

enum class PrimitiveType {
	Cube,
	Sphere,
	Plane,
};

namespace Engine::Settings {
	inline uint32_t WIDTH = 1280;
	inline uint32_t HEIGHT = 720;

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	inline uint32_t currentFrame = 0;

	inline const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	inline const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,

		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,

		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
		VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
	};

#ifdef NDEBUG
	inline const bool enableValidationLayers = false;
#else
	inline const bool enableValidationLayers = true;
#endif

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
};

namespace Engine::Utility {
	inline const std::vector<std::string> imageFileTypes = {
		".png", ".jpg",
	};
	
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
	bool hasStencilComponent(VkFormat format);
	std::vector<std::string> getAllPathsFromPath(const std::string& path, std::string ext);
	std::vector<std::string> getAllPathsFromPath(const std::string& path, std::vector<std::string> exts);
	VkTransformMatrixKHR convertMat4ToTransformMatrix(glm::mat4 mat);
	glm::mat4 convertTransformMatrixToMat4(VkTransformMatrixKHR mat);
	void setDebugName(VkDevice device, uint64_t handle, VkObjectType type, const std::string& name);

}
#endif