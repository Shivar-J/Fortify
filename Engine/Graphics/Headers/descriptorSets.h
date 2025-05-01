#ifndef DESCRIPTORSETS_H
#define DESCRIPTORSETS_H

#include "utility.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

enum class PBRTextureType {
	Albedo = 1,
	Normal = 2,
	Roughness = 3,
	Metalness = 4,
	AmbientOcclusion = 5,
	Specular = 6,
};

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
		void createDescriptorSets(VkDevice device, Engine::Graphics::Texture texture, VkDescriptorSetLayout descriptorSetLayout, bool isCube, std::unordered_map<PBRTextureType, std::string> texturePaths = {});

		VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
		std::vector<VkDescriptorSet> getDescriptorSets() const { return descriptorSets; }
	};
}
#endif