#include "utility.h"

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

            void add(const std::string& texturePath);
            void remove(int index);
            
            void pushToAccelerationStructure(std::vector<RTScene>& dst);
            void updateScene();

            std::vector<RTScene> getScenes() const { return scenes; }
            
        private:
            std::vector<RTScene> scenes;

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