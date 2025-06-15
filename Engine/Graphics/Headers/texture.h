#ifndef TEXTURE_H
#define TEXTURE_H

#include "utility.h"
#include "camera.h"

struct Materials {
	std::string name;
	std::string diffusePath;
	std::string normalPath;
	std::string roughnessPath;
	std::string metalnessPath;
	std::string aoPath;
	std::string specularPath;
};

namespace Engine::Graphics {
	struct LightBuffer {
		alignas(16) glm::vec3 pos;
		alignas(16) glm::vec3 color;
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		alignas(16) glm::vec3 color;
		alignas(16) LightBuffer lights[99];
		alignas(4) int numLights;
	};

	struct SkyboxUBO {
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Device;
	class FrameBuffer;
	class Sampler;
	class Pipeline;
	class CommandBuffer;
	class Swapchain;

	class Texture
	{
	private:
		uint32_t mipLevels;
		VkImage textureImage;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkDeviceMemory textureImageMemory;

		std::vector<VkImage> textureImages;
		std::vector<VkImageView> textureImageViews;
		std::vector<VkDeviceMemory> textureImageMemories;
		std::vector<VkSampler> textureSamples;
		std::vector<uint32_t> vecMipLevels;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		std::vector<VkBuffer> skyboxUniformBuffers;
		std::vector<VkDeviceMemory> skyboxUniformBuffersMemory;
		std::vector<void*> skyboxUniformBuffersMapped;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Materials> mats;

		std::vector<CubeVertex> cubeVertices;
		std::vector<uint32_t> cubeIndices;
		
	public:
		void createTextureImage(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR = false);
		void createTextureImageView(Engine::Graphics::Swapchain swapchain, VkDevice device, bool isCube, bool isPBR = false);
		void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice, bool isCube, bool isPBR = false);
		void loadModel(const std::string modelPath);
		void loadModel(const std::string modelPath, const std::string materialPath);
		void createVertexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb);
		void createIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb);
		void createUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb);
		void createSyncObjects(VkDevice device);
		void updateUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent, glm::mat4 model, glm::vec3 color, std::vector<LightBuffer> lights);

		void createCubemap(const std::vector<std::string>& faces, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuffer, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture);
		void createCube();
		void createSkybox();
		void createPlane();
		void createSphere(float radius=1.0f, int stacks=50, int sectors=50);
		void createCubeVertexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb);
		void createCubeIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb);
		void createSkyboxUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer);
		void updateSkyboxUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent);

		int getTextureCount() const {
			return textureImageViews.size() > 0 ? static_cast<int>(textureImageViews.size()) : -1;
		}
		uint32_t getMipLevels() const { return mipLevels; }
		VkImage getTextureImage() const { return textureImage; }
		VkImageView getTextureImageView() const { return textureImageView; }
		VkSampler getTextureSampler() const { return textureSampler; }
		VkDeviceMemory getTextureImageMemory() const { return textureImageMemory; }

		uint32_t getMipLevels(int index) const { return vecMipLevels[index]; }
		VkImage getTextureImage(int index) const { return textureImages[index]; }
		VkImageView getTextureImageView(int index) const { return textureImageViews[index]; }
		VkSampler getTextureSampler(int index) const { return textureSamples[index]; }
		VkDeviceMemory getTextureImageMemory(int index) const { return textureImageMemories[index]; }

		//std::vector<VkDeviceMemory> getTextureImageMemories() const { return textureImageMemories; }

		VkBuffer getVertexBuffer() const { return vertexBuffer; } 
		VkDeviceMemory getVertexBufferMemory() const { return vertexBufferMemory; }
		VkBuffer getIndexBuffer() const { return indexBuffer; }
		VkDeviceMemory getIndexBufferMemory() const { return indexBufferMemory; }

		std::vector<VkBuffer> getUniformBuffers() const { return uniformBuffers; }
		std::vector<VkDeviceMemory> getUniformBuffersMemory() const { return uniformBuffersMemory; }
		std::vector<void*> getUniformBuffersMapped() const { return uniformBuffersMapped; }

		std::vector<VkBuffer> getSkyboxUniformBuffers() const { return skyboxUniformBuffers; }
		std::vector<VkDeviceMemory> getSkyboxUniformBuffersMemory() const { return skyboxUniformBuffersMemory; }
		std::vector<void*> getSkyboxUniformBuffersMapped() const { return skyboxUniformBuffersMapped; }

		std::vector<VkSemaphore> getImageAvailableSemaphores() { return imageAvailableSemaphores; }
		std::vector<VkSemaphore> getRenderFinishedSemaphores() { return renderFinishedSemaphores; }
		std::vector<VkFence> getInFlightFences() { return inFlightFences; }
		
		std::vector<Vertex> getVertices() const { return vertices; }
		std::vector<uint32_t> getIndices() const { return indices; }
		std::vector<Materials> getMaterials() const { return mats; }

		std::vector<CubeVertex> getCubeVertices() const { return cubeVertices; }
		std::vector<uint32_t> getCubeIndices() const { return cubeIndices; }
	};
}

#endif // !TEXTURE_H