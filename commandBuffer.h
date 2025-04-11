#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "device.h"

namespace Engine::Graphics {
	class CommandBuffer
	{
	public:
		inline static VkCommandPool commandPool;
		inline static std::vector<VkCommandBuffer> commandBuffers;

	public:
		static void createCommandPool();
		static VkCommandBuffer beginSingleTimeCommands();
		static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		static void createCommandBuffers();
		static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	};
}
#endif // !COMMANDBUFFER_H