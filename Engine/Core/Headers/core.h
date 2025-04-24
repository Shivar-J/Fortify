#ifndef CORE_H
#define CORE_H
#include "utility.h"
#include "descriptorSets.h"
#include "texture.h"
#include "frameBuffer.h"
#include "commandBuffer.h"
#include "pipeline.h"
#include "renderPass.h"
#include "device.h"
#include "Instance.h"
#include "sampler.h"
#include "swapchain.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

enum class ModelType {
	Object,
	Skybox,
	UI,
	Transparent,
	Shadow,
	Light,
	Terrain,
	Particle
};

struct Model {
	Engine::Graphics::Pipeline pipeline;
	Engine::Graphics::Texture texture;
	Engine::Graphics::DescriptorSets descriptor;
	uint32_t indexCount;
	glm::mat4 modelMatrix;
	int32_t pipelineIndex;
	ModelType type;
};

namespace Engine::Core {
	class Application {
	private:
		Engine::Graphics::Instance instance;
		Engine::Graphics::Device device;
		Engine::Graphics::Sampler sampler;
		Engine::Graphics::Swapchain swapchain;
		Engine::Graphics::RenderPass renderpass;
		std::vector<Engine::Graphics::Pipeline> pipelines;
		Engine::Graphics::CommandBuffer commandbuffer;
		Engine::Graphics::FrameBuffer framebuffer;
		Engine::Graphics::Texture texture;
		Engine::Graphics::DescriptorSets descriptor;
		std::vector<Model> models;

	public:
		void run();
		void initWindow();
		void initVulkan();
		void initImGui();
		void mainLoop();
		void cleanup();
		void drawFrame();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void processInput(GLFWwindow* window);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

		void recreateSwapchain();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		GLFWwindow* getWindow() const { return window; }
	public:
		bool framebufferResized = false;
		inline static uint32_t currentFrame = 0;
		float deltaTime = 0.0f;
		float fpsCounter = 0.0f;		
		inline static float lastX = 0.0f;
		inline static float lastY = 0.0f;
		inline static bool firstMouse = true;

		inline static Engine::Core::Camera camera;

	private:
		GLFWwindow* window;
		VkDescriptorPool imguiPool;
	};
}
#endif
