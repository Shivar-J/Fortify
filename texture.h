#pragma once
#define GLM_FORCE_CXX17
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "frameBuffer.h"
#include "imageView.h"
#include "sampler.h"
#include "pipeline.h"

#include <unordered_map>
#include <chrono>

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
		static void updateUniformBuffer(uint32_t currentImage);
	};
}
