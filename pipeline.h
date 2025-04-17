#ifndef PIPELINE_H
#define PIPELINE_H

#include "utility.h"

namespace Engine::Graphics {
	class Device;
	class RenderPass;
	class Sampler;

	class Pipeline
	{
	private:
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
	public:
		Pipeline() = default;
		~Pipeline();

		void createGraphicsPipeline(std::string vertexShaderPath, std::string fragmentShaderPath, VkDevice device, VkSampleCountFlagBits msaaSamples, Engine::Graphics::RenderPass renderpass);
		VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
		std::vector<char> readFile(const std::string& filename);

		VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
		VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
	};
}
#endif
