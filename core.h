#ifndef CORE_H
#define CORE_H
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
#include "camera.h"

inline static Engine::Core::Camera camera(glm::vec3(0.0f));

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
		static void processInput(GLFWwindow* window);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

	public:
		inline static GLFWwindow* window;
		inline static bool framebufferResized = false;
		inline static uint32_t currentFrame = 0;
		inline static float deltaTime = 0.0f;
		inline static float fpsCounter = 0.0f;		inline static float lastX = 0.0f;
		inline static float lastY = 0.0f;
		inline static bool firstMouse = true;
	};
}
#endif
