#ifndef PIPELINE_H
#define PIPELINE_H

#include "utility.h"
#include "device.h"
#include "renderPass.h"

namespace Engine::Graphics {
	class Pipeline
	{
	public:
		inline static VkPipelineLayout pipelineLayout;
		inline static VkPipeline graphicsPipeline;
	public:
		static void createGraphicsPipeline();
		static VkShaderModule createShaderModule(const std::vector<char>& code);
		static std::vector<char> readFile(const std::string& filename);
	};
}
#endif
