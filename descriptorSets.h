#ifndef DESCRIPTORSETS_H
#define DESCRIPTORSETS_H

#include "utility.h"

namespace Engine::Graphics {
	class Texture;
	class Device;
	class RenderPass;

	class DescriptorSets
	{
	private:
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

	public:
		void createDescriptorPool(VkDevice device);
		void createDescriptorSets(VkDevice device, Engine::Graphics::Texture texture, VkDescriptorSetLayout descriptorSetLayout);

		VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
		std::vector<VkDescriptorSet> getDescriptorSets() const { return descriptorSets; }
	};
}
#endif