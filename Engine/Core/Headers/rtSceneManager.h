#include "utility.h"
#include "sceneUtility.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "imfilebrowser.h"

namespace Engine::Graphics {
    class Device;
    class Sampler;
    class CommandBuffer;
    class FrameBuffer;
    class Swapchain;
    class Texture;
    class Raytracing;
    class Animation;
}

namespace Engine::Core {
    class Camera;
    namespace RT {
        class SceneManager {
        public:
            SceneManager(
                Engine::Graphics::Device& device,
                Engine::Graphics::Sampler& sampler,
                Engine::Graphics::CommandBuffer& commandbuffer,
                Engine::Graphics::FrameBuffer& framebuffer,
                Engine::Graphics::Swapchain& swapchain,
                Engine::Core::Camera& camera,
                Engine::Graphics::Texture& texture,
                Engine::Graphics::Raytracing& raytrace
            ) : device(device), sampler(sampler), commandbuffer(commandbuffer), framebuffer(framebuffer), swapchain(swapchain), camera(camera), texture(texture), raytrace(raytrace) {};

            void add(const std::string& texturePath, bool flipTexture = false);
            void remove(int index);
            
            void pushToAccelerationStructure(std::vector<std::shared_ptr<RTScene>>& dst);
            void updateScene(float deltaTime);

            std::vector<std::shared_ptr<RTScene>> getScenes() const { return scenes; }
            
        private:
            std::vector<std::shared_ptr<RTScene>> scenes;

            Engine::Graphics::Device& device;
            Engine::Graphics::Sampler& sampler;
            Engine::Graphics::CommandBuffer& commandbuffer;
            Engine::Graphics::FrameBuffer& framebuffer;
            Engine::Graphics::Swapchain& swapchain;
            Engine::Core::Camera& camera;
            Engine::Graphics::Texture& texture;
            Engine::Graphics::Raytracing& raytrace;
        };
    }
}