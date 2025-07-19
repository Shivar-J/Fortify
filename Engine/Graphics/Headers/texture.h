#ifndef TEXTURE_H
#define TEXTURE_H

#include "utility.h"
#include "camera.h"

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
	public:
		ImageResource* textureResource;
		std::vector<ImageResource*> textureResources;

		BufferResource* vertexResource;
		BufferResource* indexResource;

		std::vector<BufferResource*> uniformResources;
		std::vector<BufferResource*> skyboxUniformResources;

	private:
		uint32_t mipLevels;
		std::vector<uint32_t> vecMipLevels;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Materials> mats;

		std::vector<CubeVertex> cubeVertices;
		std::vector<uint32_t> cubeIndices;
		
	public:
		void createTextureImage(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR = false, bool isCube = false, bool useSampler = false);
		ImageResource* createImageResource(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR = false, bool isCube = false, bool useSampler = false);
		void loadModel(const std::string modelPath);
		void loadModel(const std::string modelPath, const std::string materialPath);
		MeshObject loadModelRT(const std::string modelPath, Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb);
		MeshObject loadModelRT(const std::string modelPath, const std::string materialPath, Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb, Engine::Graphics::CommandBuffer cb, Engine::Graphics::Sampler sampler, Engine::Graphics::Swapchain swapchain);
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

		void cleanup(VkDevice device);

		int getTextureCount() const {
			return textureResources.size() > 0 ? static_cast<int>(textureResources.size()) : -1;
		}
		uint32_t getMipLevels() const { return mipLevels; }
		uint32_t getMipLevels(int index) const { return vecMipLevels[index]; }

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