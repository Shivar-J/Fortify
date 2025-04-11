#ifndef TEXTURE_H
#define TEXTURE_H

#include "frameBuffer.h"
#include "imageView.h"
#include "sampler.h"
#include "pipeline.h"
#include "utility.h"
#include "camera.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
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
		attributeDescription[1].offset = offsetof(Vertex, color);

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
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace Engine::Graphics {
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Texture
	{
	public:
		inline static uint32_t mipLevels;
		inline static VkImage textureImage;
		inline static VkImageView textureImageView;
		inline static VkSampler textureSampler;
		inline static VkDeviceMemory textureImageMemory;

		inline static VkBuffer vertexBuffer;
		inline static VkDeviceMemory vertexBufferMemory;
		inline static VkBuffer indexBuffer;
		inline static VkDeviceMemory indexBufferMemory;

		inline static std::vector<VkBuffer> uniformBuffers;
		inline static std::vector<VkDeviceMemory> uniformBuffersMemory;
		inline static std::vector<void*> uniformBuffersMapped;

		inline static std::vector<VkSemaphore> imageAvailableSemaphores;
		inline static std::vector<VkSemaphore> renderFinishedSemaphores;
		inline static std::vector<VkFence> inFlightFences;

		inline static std::vector<Vertex> vertices;
		inline static std::vector<uint32_t> indices;
	public:
		static void createTextureImage();
		static void createTextureImageView();
		static void createTextureSampler();
		static void loadModel();
		static void createVertexBuffer();
		static void createIndexBuffer();
		static void createUniformBuffers();
		static void createSyncObjects();
		static void updateUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera);
	};
}

#endif // !TEXTURE_H