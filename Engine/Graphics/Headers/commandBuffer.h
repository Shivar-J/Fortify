#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "utility.h"

namespace Engine::Graphics {
	class Device;
	class Pipeline;
	class RenderPass;
	class Texture;
	class Swapchain;
	class DescriptorSets;

	class CommandBuffer
	{
	private:
		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

	public:
		CommandBuffer() = default;
		~CommandBuffer();

		void createCommandPool(Engine::Graphics::Device device, VkSurfaceKHR surface);
		VkCommandBuffer beginSingleTimeCommands(VkDevice device);
		void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkDevice device);
		void createCommandBuffers(VkDevice device);

		void transitionImageLayout(const Engine::Graphics::Device& device, ImageResource* image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);

		void copyBufferToImage(const Engine::Graphics::Device& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
		void copyBuffer(const Engine::Graphics::Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		VkCommandPool getCommandPool() const { return commandPool; }
		const std::vector<VkCommandBuffer>& getCommandBuffers() const { return commandBuffers; }

	};
}
#endif