#ifndef SAMPLER_H
#define SAMPLER_H

#include "utility.h"

namespace Engine::Graphics {
	class Device;
	class CommandBuffer;

	class Sampler
	{
	public:
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_16_BIT;

	public:
		Sampler() = default;
		~Sampler();

		void setSamples(VkPhysicalDevice physicalDevice);
		VkSampleCountFlagBits getSamples() const { return msaaSamples; }

		VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
		void generateMipmaps(Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::Device device, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	};
}

#endif
