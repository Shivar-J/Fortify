#ifndef CORE_H
#define CORE_H

#include "utility.h"
#include "vulkanPointers.hpp"
#include "raytracing.h"
#include "sceneManager.h"
#include "frameBuffer.h"
#include "commandBuffer.h"
#include "renderPass.h"
#include "device.h"
#include "instance.h"
#include "sampler.h"
#include "swapchain.h"
#include "camera.h"
#include "rtSceneManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "sceneUtility.h"

namespace Engine::Core {
	class Application {
	public:
		Application() :
			scenemanager(device, sampler, renderpass, commandbuffer, framebuffer, swapchain, camera, raytrace),
			rtscenemanager(device, sampler, commandbuffer, framebuffer, swapchain, camera, texture, raytrace)
		{}

		void run();
		void initWindow();
		void initVulkan();
		void initImGui();
		void mainLoop();
		void cleanup();
		void drawFrame();
		void raytraceFrame();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void processInput(GLFWwindow* window);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

		void recreateSwapchain();
		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void createImGuiRenderPass();
		void createImGuiFramebuffers();

		GLFWwindow* getWindow() const { return window; }
	public:
		inline static bool framebufferResized = false;
		inline static uint32_t currentFrame = 0;
		float deltaTime = 0.0f;
		float fpsCounter = 0.0f;		
		inline static float lastX = 0.0f;
		inline static float lastY = 0.0f;
		inline static bool firstMouse = true;
		inline static bool isFocused = true;

		inline static Engine::Core::Camera camera;

	private:
		GLFWwindow* window = nullptr;
		VkDescriptorPool imguiPool = VK_NULL_HANDLE;
		VkRenderPass imguiRenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> imguiFramebuffers;
		bool accumulateFrames = true;
		bool fullscreen = false;
		bool recreateSwapchainFlag = false;

		Engine::Graphics::Instance instance;
		Engine::Graphics::Device device;
		Engine::Graphics::Sampler sampler;
		Engine::Graphics::Swapchain swapchain;
		Engine::Graphics::RenderPass renderpass;
		Engine::Graphics::CommandBuffer commandbuffer;
		Engine::Graphics::FrameBuffer framebuffer;
		Engine::Graphics::Texture texture;
		Engine::Graphics::DescriptorSets descriptor;
		Engine::Core::SceneManager scenemanager;
		Engine::Core::RT::SceneManager rtscenemanager;
		Engine::Graphics::Raytracing raytrace;
		Engine::Graphics::Texture skyboxTexture;
	};
}
#endif
