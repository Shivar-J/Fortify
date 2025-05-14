#include "texture.h"
#include "commandBuffer.h"
#include "renderPass.h"
#include "frameBuffer.h"
#include "device.h"
#include "swapchain.h"
#include "sampler.h"

void Engine::Graphics::Texture::createTextureImage(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR)
{
	if (flipTexture) {
		stbi_set_flip_vertically_on_load(true);
	}
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels)
		throw std::runtime_error("failed to load texture image");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	framebuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device.getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	framebuffer.createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 1, 0);
	commandBuf.transitionImageLayout(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
	commandBuf.copyBufferToImage(device, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

	sampler.generateMipmaps(commandBuf, device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1);
    
    if (isPBR) {
        textureImages.push_back(textureImage);
        textureImageMemories.push_back(textureImageMemory);
        vecMipLevels.push_back(mipLevels);
    }

    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createTextureImageView(Engine::Graphics::Swapchain swapchain, VkDevice device, bool isCube, bool isPBR)
{
    textureImageView = swapchain.createImageView(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, isCube);

    if (isPBR) {
        textureImageViews.push_back(textureImageView);
    }
}

void Engine::Graphics::Texture::createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice, bool isCube, bool isPBR)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    if (isCube) {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
    }
    else {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
    }
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    
    if (isPBR) {
        textureSamples.push_back(textureSampler);
    }
}

void Engine::Graphics::Texture::loadModel(const std::string modelPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = std::filesystem::path(modelPath).parent_path().string();

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        int face = 0;
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void Engine::Graphics::Texture::loadModel(const std::string modelPath, const std::string materialPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir = std::filesystem::path(materialPath).parent_path().string();

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), baseDir.c_str())) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& mat : materials) {
        Materials material;

        material.name = mat.name;
        material.diffusePath = mat.diffuse_texname;
        material.normalPath = mat.normal_texname;
        material.roughnessPath = mat.roughness_texname;
        material.metalnessPath = mat.metallic_texname;
        material.aoPath = mat.ambient_texname;
        material.specularPath = mat.specular_texname;

        mats.push_back(material);
    }


    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        int face = 0;
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void Engine::Graphics::Texture::createVertexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device.getDevice(), stagingBufferMemory);

    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    commandBuf.copyBuffer(device, stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device.getDevice(), stagingBufferMemory);

    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    commandBuf.copyBuffer(device, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device.getDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void Engine::Graphics::Texture::createSyncObjects(VkDevice device)
{
    imageAvailableSemaphores.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Engine::Graphics::Texture::updateUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent)
{
    camera.AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Engine::Graphics::Texture::updateUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent, glm::mat4 model)
{
    camera.AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    UniformBufferObject ubo{};
    ubo.model = model;
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Engine::Graphics::Texture::createCubemap(const std::vector<std::string>& faces, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture)
{
    if (flipTexture) {
        stbi_set_flip_vertically_on_load(true);
    }

    int texWidth, texHeight, texChannels;
    VkDeviceSize layerSize;

    std::vector<stbi_uc*> pixels(6);
    for (size_t i = 0; i < 6; i++) {
        pixels[i] = stbi_load(faces[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels[i]) {
            throw std::runtime_error("failed to load image: " + faces[i]);
        }
    }

    layerSize = texWidth * texHeight * 4;
    VkDeviceSize imageSize = layerSize * 6;

    mipLevels = 1;
    //mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    framebuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    for (size_t i = 0; i < 6; i++) {
        memcpy(static_cast<char*>(data) + (layerSize * i), pixels[i], static_cast<size_t>(layerSize));
        stbi_image_free(pixels[i]);
    }

    vkUnmapMemory(device.getDevice(), stagingBufferMemory);

    framebuffer.createImage(device, texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    
    commandBuf.transitionImageLayout(device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 6);
    commandBuf.copyBufferToImage(device, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);
    sampler.generateMipmaps(commandBuf, device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 6);
    
    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createCube()
{
    vertices = {
        {{-1.0f, -1.0f,  1.0f}, {0.7f, 0.7f, 0.7f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.7f, 0.7f, 0.7f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.7f, 0.7f, 0.7f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {0.7f, 0.7f, 0.7f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.7f, 0.7f, 0.7f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {0.7f, 0.7f, 0.7f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.7f, 0.7f, 0.7f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.7f, 0.7f, 0.7f}, {0.0f, 1.0f}}
    };

    indices = {
        0, 1, 2, 2, 3, 0,
        5, 4, 7, 7, 6, 5,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1
    };
}

void Engine::Graphics::Texture::createPlane() {
    vertices = {
        {{-1.0f, -1.0, 1.0f}, {0.7f, 0.7f, 0.7f}, {0.0, 0.0}},
        {{ 1.0f, -1.0, 1.0f}, {0.7f, 0.7f, 0.7f}, {1.0, 0.0}},
        {{ 1.0f,  1.0, 1.0f}, {0.7f, 0.7f, 0.7f}, {1.0, 1.0}},
        {{-1.0f,  1.0, 1.0f}, {0.7f, 0.7f, 0.7f}, {0.0, 1.0}},
    };

    indices = {
        0, 1, 2,
        2, 3, 0
    };
}

void Engine::Graphics::Texture::createSphere(float radius, int stacks, int sectors) {
    float pi = 3.14159265358979323846f;

    for (uint32_t i = 0; i <= stacks; i++) {
        float phi = pi * float(i) / float(stacks);
        float y = radius * cosf(phi);
        float r = radius * sinf(phi);
        for (uint32_t j = 0; j <= sectors; j++) {
            float theta = 2 * pi * float(j) / float(sectors);
            float x = radius * cosf(theta);
            float z = radius * sinf(theta);

            Vertex vertex{};
            vertex.pos = {
                radius * sinf(phi) * cosf(theta),
                radius * cosf(phi),
                radius * sinf(phi) * sinf(theta)
            };

            vertex.texCoord = {
                float(j) / float(sectors),
                float(i) / float(stacks)
            };

            vertex.color = { 0.7f, 0.7f, 0.7f };

            vertices.push_back(vertex);
        }
    }

    for (uint32_t i = 0; i < stacks; i++) {
        for (uint32_t j = 0; j < sectors; j++) {
            uint32_t first = i * (sectors + 1) + j;
            uint32_t second = first + sectors + 1;

            if(i != stacks - 1)
            {
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(first + 1);
                indices.push_back(second);
                indices.push_back(second + 1);
            }
        }
    }

    uint32_t bottomStart = (stacks - 1) * (sectors + 1);
    uint32_t bottomCenterIndex = (uint32_t)vertices.size();
    Vertex vertex{};

    vertex.pos = {
        0, -radius, 0
    };

    vertex.texCoord = {
        0.5, 1.0
    };

    vertex.color = {
        0.7, 0.7, 0.7
    };
    
    vertices.push_back(vertex);

    for (uint32_t j = 0; j < sectors; j++) {
        uint32_t first = bottomStart + j;
        uint32_t second = bottomStart + ((j + 1) % sectors);
        indices.push_back(first);
        indices.push_back(bottomCenterIndex);
        indices.push_back(second);
    }
}

void Engine::Graphics::Texture::createSkybox()
{
    cubeVertices = {
        {{-1.0f, -1.0f,  1.0f}},
        {{ 1.0f, -1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}},
        {{-1.0f,  1.0f,  1.0f}},
        {{-1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f,  1.0f, -1.0f}},
        {{-1.0f,  1.0f, -1.0f}}
    };

    cubeIndices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        5, 4, 7, 7, 6, 5,
        // Top
        3, 2, 6, 6, 7, 3,
        // Bottom
        4, 5, 1, 1, 0, 4,
        // Left
        4, 0, 3, 3, 7, 4,
        // Right
        1, 5, 6, 6, 2, 1
    };
}

void Engine::Graphics::Texture::createCubeVertexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb) {
    VkDeviceSize bufferSize = sizeof(cubeVertices[0]) * cubeVertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    fb.createBuffer(
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, cubeVertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device.getDevice(), stagingBufferMemory);

    fb.createBuffer(
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory
    );

    commandBuf.copyBuffer(device, stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createCubeIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(cubeIndices[0]) * cubeIndices.size();
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, cubeIndices.data(), (size_t)bufferSize);
    vkUnmapMemory(device.getDevice(), stagingBufferMemory);

    fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    commandBuf.copyBuffer(device, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
}

void Engine::Graphics::Texture::createSkyboxUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer)
{
    VkDeviceSize bufferSize = sizeof(SkyboxUBO);

    skyboxUniformBuffers.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    skyboxUniformBuffersMemory.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    skyboxUniformBuffersMapped.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        framebuffer.createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, skyboxUniformBuffers[i], skyboxUniformBuffersMemory[i]);

        vkMapMemory(device.getDevice(), skyboxUniformBuffersMemory[i], 0, bufferSize, 0, &skyboxUniformBuffersMapped[i]);
    }
}

void Engine::Graphics::Texture::updateSkyboxUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent) {
    camera.AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    SkyboxUBO skyboxUBO{};
    skyboxUBO.view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    skyboxUBO.proj = camera.GetProjectionMatrix();

    memcpy(skyboxUniformBuffersMapped[currentImage], &skyboxUBO, sizeof(skyboxUBO));
}