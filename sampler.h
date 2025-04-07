#pragma once
#include "device.h"
#include "commandBuffer.h"

namespace Engine::Graphics {
	class Sampler
	{
	public:
		inline static VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_16_BIT;

	public:
		static VkSampleCountFlagBits getMaxUsableSampleCount();
		static void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	};
}


