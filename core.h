#pragma once
#include "utility.h"
#include "descriptorSets.h"
#include "texture.h"
#include "frameBuffer.h"
#include "commandBuffer.h"
#include "pipeline.h"
#include "renderPass.h"
#include "imageView.h"
#include "device.h"
#include "Instance.h"
#include "sampler.h"
#include "swapchain.h"
#include <cstdio>

namespace Engine::Core {
	class Application {
	public:
		void run();
		void initWindow();
		void initVulkan();
		void mainLoop();
		void cleanup();
		void drawFrame();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:
		inline static GLFWwindow* window;
		inline static bool framebufferResized = false;
		inline static uint32_t currentFrame = 0;
	};
}

