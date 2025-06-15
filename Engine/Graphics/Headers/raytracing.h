#ifndef RAYTRACING_H
#define RAYTRACING_H

#include "utility.h"
#include "vulkanPointers.hpp"

struct AccelerationStructure {
	VkAccelerationStructureKHR handle;
	VkDeviceMemory memory;
	VkBuffer buffer;
	VkDeviceAddress deviceAddress;

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
};

struct RaytracingUniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    uint32_t vertexSize;
    uint32_t sampleCount = 0;
    uint32_t samplesPerFrame = 4;
    uint32_t rayBounces = 8;
};

struct ModelGeom {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
};

namespace Engine::Graphics {
	class FrameBuffer;
	class Device;
    class Texture;
    class CommandBuffer;

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
        
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;

        VkBuffer raygenSBTBuffer;
        VkDeviceMemory raygenSBTMemory;

        VkBuffer missSBTBuffer;
        VkDeviceMemory missSBTMemory;

        VkBuffer hitSBTBuffer;
        VkDeviceMemory hitSBTMemory;

        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        RaytracingUniformBufferObject uboData;
        void* uboMapped = nullptr;

        std::vector<ModelGeom> models;

	public:
        VkDeviceAddress getBufferDeviceAddress(VkDevice device, VkBuffer buffer);
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

        auto createBottomLevelAccelerationStructure(Engine::Graphics::Device device, uint32_t index);
        void createBottomLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, ModelGeom model);
        void createTopLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer);
        void createShaderBindingTables(Engine::Graphics::Device device);
        void createDescriptorSets(Engine::Graphics::Device device);
        void createRayTracingPipeline(Engine::Graphics::Device device, std::string raygenShaderPath, std::string missShaderPath, std::string chitShaderPath);
        void createImage(Engine::Graphics::Device device, VkCommandPool commandPool, VkExtent2D extent);
        void traceRays(VkDevice device, VkCommandBuffer commandBuffer, VkExtent2D extent, VkImage swapchainImage, uint32_t currentImageIndex);
    
        void createUniformBuffer(Engine::Graphics::Device device);
        void updateUBO(Engine::Graphics::Device device);
    };
}

#endif