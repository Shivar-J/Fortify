#include "texture.h"
#include "commandBuffer.h"
#include "renderPass.h"
#include "frameBuffer.h"
#include "device.h"
#include "swapchain.h"
#include "sampler.h"

void Engine::Graphics::Texture::createTextureImage(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR, bool isCube, bool useSampler)
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

    BufferResource* stagingBuffer = framebuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	memcpy(stagingBuffer->mapped, pixels, static_cast<size_t>(imageSize));

	stbi_image_free(pixels);

	textureResource = framebuffer.createImage(device.getDevice(), device.getPhysicalDevice(), texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_ASPECT_COLOR_BIT, isCube, useSampler);
	commandBuf.transitionImageLayout(device, textureResource, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
	commandBuf.copyBufferToImage(device, stagingBuffer->buffer, textureResource->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

	sampler.generateMipmaps(commandBuf, device, textureResource, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1);

    if (isPBR) {
        textureResources.push_back(textureResource);
        vecMipLevels.push_back(mipLevels);
    }

    resources->destroy(stagingBuffer);
}

ImageResource* Engine::Graphics::Texture::createImageResource(const std::string texturePath, Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::Sampler sampler, bool flipTexture, bool isPBR, bool isCube, bool useSampler)
{
    ImageResource* image;

    if (flipTexture) {
        stbi_set_flip_vertically_on_load(true);
    }
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
        throw std::runtime_error("failed to load texture image");

    BufferResource* stagingBuffer = framebuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->mapped, pixels, static_cast<size_t>(imageSize));

    stbi_image_free(pixels);

    image = framebuffer.createImage(device.getDevice(), device.getPhysicalDevice(), texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 0, VK_IMAGE_ASPECT_COLOR_BIT, isCube, useSampler);
    commandBuf.transitionImageLayout(device, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
    commandBuf.copyBufferToImage(device, stagingBuffer->buffer, image->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);

    sampler.generateMipmaps(commandBuf, device, image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1);

    if (isPBR) {
        textureResources.push_back(image);
        vecMipLevels.push_back(mipLevels);
    }

    resources->destroy(stagingBuffer);

    return image;
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

            if (index.normal_index > 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            else {
                vertex.normal = { 0.0, 1.0, 0.0 };
            }

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

            if (index.normal_index > 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            else {
                vertex.normal = { 0.0, 1.0, 0.0 };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

MeshObject Engine::Graphics::Texture::loadModelRT(const std::string modelPath, Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb)
{
    MeshObject t;

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

            if (index.normal_index > 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            else {
                vertex.normal = { 0.0, 1.0, 0.0 };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(t.v.size());
                t.v.push_back(vertex);
            }

            t.i.push_back(uniqueVertices[vertex]);
        }
    }


    VkDeviceSize vertexBufferSize = sizeof(t.v[0]) * t.v.size();
    t.vertex = fb.createBuffer(device, vertexBufferSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, t.v.data());


    VkDeviceSize indexBufferSize = sizeof(t.i[0]) * t.i.size();
    t.index = fb.createBuffer(device, indexBufferSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, t.i.data());

    t.material = nullptr;

    return t;
}

MeshObject Engine::Graphics::Texture::loadModelRT(const std::string modelPath, const std::string materialPath, Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb, Engine::Graphics::CommandBuffer cb, Engine::Graphics::Sampler sampler, Engine::Graphics::Swapchain swapchain)
{
    MeshObject t;

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

        t.m.push_back(material);
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

            if (index.normal_index > 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            else {
                vertex.normal = { 0.0, 1.0, 0.0 };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(t.v.size());
                t.v.push_back(vertex);
            }

            t.i.push_back(uniqueVertices[vertex]);
        }
    }

    VkDeviceSize vertexBufferSize = sizeof(t.v[0]) * t.v.size();
    t.vertex = fb.createBuffer(device, vertexBufferSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, t.v.data());

    VkDeviceSize indexBufferSize = sizeof(t.i[0]) * t.i.size();
    t.index = fb.createBuffer(device, indexBufferSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, t.i.data());

    VkDeviceSize matBufferSize = sizeof(t.m[0]) * t.m.size();
    t.material = fb.createBuffer(device, matBufferSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, t.m.data());

    return t;
}

void Engine::Graphics::Texture::createVertexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    BufferResource* stagingBuffer = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->mapped, vertices.data(), (size_t)bufferSize);

    vertexResource = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    commandBuf.copyBuffer(device, stagingBuffer->buffer, vertexResource->buffer, bufferSize);

    resources->destroy(stagingBuffer);
}

void Engine::Graphics::Texture::createIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    BufferResource* stagingBuffer = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->mapped, indices.data(), (size_t)bufferSize);

    indexResource = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    commandBuf.copyBuffer(device, stagingBuffer->buffer, indexResource->buffer, bufferSize);

    resources->destroy(stagingBuffer);
}

void Engine::Graphics::Texture::createUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformResources.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        uniformResources[i] = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

void Engine::Graphics::Texture::updateUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent, glm::mat4 model, glm::vec3 color, std::vector<LightBuffer> lights)
{
    camera.AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    UniformBufferObject ubo{};
    ubo.model = model;
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();
    ubo.color = color;
    ubo.numLights = std::min(static_cast<int>(lights.size()), 99);

    for (int i = 0; i < ubo.numLights; i++) {
        ubo.lights[i] = lights[i];
    }
         
    memcpy(uniformResources[currentImage]->mapped, &ubo, sizeof(ubo));
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

    BufferResource* stagingBuffer = framebuffer.createBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    for (size_t i = 0; i < 6; i++) {
        memcpy(static_cast<char*>(stagingBuffer->mapped) + (layerSize * i), pixels[i], static_cast<size_t>(layerSize));
        stbi_image_free(pixels[i]);
    }

    textureResource = framebuffer.createImage(device.getDevice(), device.getPhysicalDevice(), texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, true, true);
    
    commandBuf.transitionImageLayout(device, textureResource, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 6);
    commandBuf.copyBufferToImage(device, stagingBuffer->buffer, textureResource->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);
    sampler.generateMipmaps(commandBuf, device, textureResource, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 6);
    
    resources->destroy(stagingBuffer);
}

void Engine::Graphics::Texture::createCube()
{
    vertices = {
        {{-1.0f, -1.0f,  1.0f}, {}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  1.0f}, {}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f}, {}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {}, {0.0f, 1.0f}}
    };

    indices = {
        0, 1, 2, 2, 3, 0,
        5, 4, 7, 7, 6, 5,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1
    };

    for (auto& v : vertices) {
        v.normal = { 0.0, 0.0, 0.0 };
    }

    for (uint32_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        glm::vec3 edge1 = vertices[i1].pos - vertices[i0].pos;
        glm::vec3 edge2 = vertices[i2].pos - vertices[i0].pos;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    for (auto& v : vertices) {
        v.normal = glm::normalize(v.normal);
    }
}

void Engine::Graphics::Texture::createPlane() {
    vertices = {
        {{-1.0f, -1.0, 1.0f}, {}, {0.0, 0.0}},
        {{ 1.0f, -1.0, 1.0f}, {}, {1.0, 0.0}},
        {{ 1.0f,  1.0, 1.0f}, {}, {1.0, 1.0}},
        {{-1.0f,  1.0, 1.0f}, {}, {0.0, 1.0}},
    };

    indices = {
        0, 1, 2,
        2, 3, 0
    };

    for (auto& v : vertices) {
        v.normal = { 0.0, 0.0, 0.0 };
    }

    for (uint32_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        glm::vec3 edge1 = vertices[i1].pos - vertices[i0].pos;
        glm::vec3 edge2 = vertices[i2].pos - vertices[i0].pos;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    for (auto& v : vertices) {
        v.normal = glm::normalize(v.normal);
    }
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
            glm::vec3 pos = {
                radius * sinf(phi) * cosf(theta),
                radius * cosf(phi),
                radius * sinf(phi) * sinf(theta)
            };

            vertex.pos = pos;
            vertex.normal = glm::normalize(pos);

            vertex.texCoord = {
                float(j) / float(sectors),
                float(i) / float(stacks)
            };

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

    vertex.normal = {
        0, -1, 0
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

    BufferResource* stagingBuffer = fb.createBuffer(
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    memcpy(stagingBuffer->mapped, cubeVertices.data(), (size_t)bufferSize);

    vertexResource = fb.createBuffer(
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    commandBuf.copyBuffer(device, stagingBuffer->buffer, vertexResource->buffer, bufferSize);

    resources->destroy(stagingBuffer);
}

void Engine::Graphics::Texture::createCubeIndexBuffer(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandBuf, Engine::Graphics::FrameBuffer fb)
{
    VkDeviceSize bufferSize = sizeof(cubeIndices[0]) * cubeIndices.size();
    
    BufferResource* stagingBuffer = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    memcpy(stagingBuffer->mapped, cubeIndices.data(), (size_t)bufferSize);

    indexResource = fb.createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    commandBuf.copyBuffer(device, stagingBuffer->buffer, indexResource->buffer, bufferSize);

    resources->destroy(stagingBuffer);
}

void Engine::Graphics::Texture::createSkyboxUniformBuffers(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer)
{
    VkDeviceSize bufferSize = sizeof(SkyboxUBO);

    skyboxUniformResources.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        skyboxUniformResources[i] = framebuffer.createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(device.getDevice(), skyboxUniformResources[i]->memory, 0, bufferSize, 0, &skyboxUniformResources[i]->mapped);
    }
}

void Engine::Graphics::Texture::updateSkyboxUniformBuffer(uint32_t currentImage, Engine::Core::Camera& camera, VkExtent2D swapChainExtent) {
    camera.AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
    SkyboxUBO skyboxUBO{};
    skyboxUBO.view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    skyboxUBO.proj = camera.GetProjectionMatrix();

    memcpy(skyboxUniformResources[currentImage]->mapped, &skyboxUBO, sizeof(skyboxUBO));
}

void Engine::Graphics::Texture::cleanup(VkDevice device) {
    resources->destroy(textureResource);

    for (size_t i = 0; i < textureResources.size(); i++) {
        resources->destroy(textureResources[i]);
    }

    for (auto sem : imageAvailableSemaphores)
        if (sem) vkDestroySemaphore(device, sem, nullptr);
    for (auto sem : renderFinishedSemaphores)
        if (sem) vkDestroySemaphore(device, sem, nullptr);
    for (auto fence : inFlightFences)
        if (fence) vkDestroyFence(device, fence, nullptr);

    for (size_t i = 0; i < uniformResources.size(); ++i) {
        resources->destroy(uniformResources[i]);
    }
    for (size_t i = 0; i < skyboxUniformResources.size(); ++i) {
        resources->destroy(skyboxUniformResources[i]);
    }

    resources->destroy(vertexResource);
    resources->destroy(indexResource);
}