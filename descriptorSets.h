#pragma once
#include "texture.h"
#include "renderPass.h"

namespace Engine::Graphics {
	class DescriptorSets
	{
	public:
		inline static VkDescriptorPool descriptorPool;
		inline static std::vector<VkDescriptorSet> descriptorSets;

	public:
		static void createDescriptorPool();
		static void createDescriptorSets();
	};
}