#ifndef RAYTRACING_H
#define RAYTRACING_H

#include "utility.h"
#include "vulkanPointers.hpp"
#include "texture.h"
#include "sceneUtility.h"

struct AccelerationStructure {
    AccelerationStructureResource* resource;

    void create(Engine::Graphics::Device device, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
};

struct ScratchBuffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceAddress deviceAddress;

    ScratchBuffer(Engine::Graphics::Device device, VkDeviceSize size);
};

struct StorageImage {
    VkDeviceMemory memory;
    VkImage image;
    VkImageView view;
    VkFormat format;

    void create(Engine::Graphics::Device device, VkQueue queue, VkCommandPool commandPool, VkFormat format, VkExtent3D extent);
    void destroy(VkDevice device);
};

struct RaytracingUniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    uint32_t vertexSize;
    uint32_t sampleCount = 1;
    uint32_t samplesPerFrame = 1;
    uint32_t rayBounces = 5;
    uint32_t numLights = 0;
    std::vector<Engine::Graphics::LightBuffer> lights;
};

namespace Engine::Core::RT {
    class SceneManager;
}

namespace Engine::Graphics {
	class FrameBuffer;
	class Device;
    class CommandBuffer;
    class Swapchain;

	class Raytracing
	{
	public :
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

        std::vector<AccelerationStructure> BLAS;
        AccelerationStructure TLAS;
        
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

        StorageImage accumulationImage;
        StorageImage storageImage;
        
        std::string raygenPath = "";
        std::string missPath = "";
        std::string cHitPath = "";
        std::string aHitPath = "";
        std::string intPath = "";

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;

        BufferResource* raygenResource;
        BufferResource* missResource;
        BufferResource* closestHitResource;
        BufferResource* anyHitResource;

        BufferResource* uniformBuffer;
        RaytracingUniformBufferObject uboData;

        BufferResource* instanceBuffer;
        BufferResource* textureFlagBuffer;

        std::vector<std::shared_ptr<RTScene>> models;

        bool sceneUpdated = false;
	public:
        VkDeviceAddress getBufferDeviceAddress(VkDevice device, VkBuffer buffer);
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

        void initRaytracing(Engine::Graphics::Device device);
        auto createBottomLevelAccelerationStructure(Engine::Graphics::Device device, uint32_t index);
        void createBottomLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, std::shared_ptr<RTScene> model);
        void createTopLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer);
        void updateTopLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, bool rebuild = false);
        
        void buildAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandbuffer, Engine::Graphics::FrameBuffer framebuffer);
        void createShaderBindingTables(Engine::Graphics::Device device);
        void createDescriptorSets(Engine::Graphics::Device device, std::optional<Engine::Graphics::Texture> skyboxTexture = std::nullopt);
        void updateDescriptorSets(Engine::Graphics::Device device);
        void createRayTracingPipeline(Engine::Graphics::Device device, std::string raygenShaderPath, std::string missShaderPath, std::string chitShaderPath, std::string ahitShaderPath, std::string intShaderPath);
        void createImage(Engine::Graphics::Device device, VkCommandPool commandPool, VkExtent2D extent);
        void traceRays(VkDevice device, VkCommandBuffer commandBuffer, SwapchainResource* resource, uint32_t currentIndex);
    
        void createUniformBuffer(Engine::Graphics::Device device);
        void updateUBO(Engine::Graphics::Device device);
        void recreateScene(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, Engine::Graphics::Swapchain swapchain, Engine::Core::RT::SceneManager rtscenemanager, std::optional<Engine::Graphics::Texture> skyboxTexture = std::nullopt);

        void cleanup(VkDevice device, bool softClean = false);
    };
}

#endif