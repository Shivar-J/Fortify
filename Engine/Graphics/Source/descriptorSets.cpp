#include "descriptorSets.h"
#include "device.h"
#include "texture.h"
#include "renderPass.h"

void Engine::Graphics::DescriptorSets::createDescriptorPool(VkDevice device)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(Engine::Settings::MAX_FRAMES_IN_FLIGHT) * 5;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(Engine::Settings::MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Engine::Graphics::DescriptorSets::createDescriptorSets(VkDevice device, Engine::Graphics::Texture texture, VkDescriptorSetLayout descriptorSetLayout, bool isCube, std::unordered_map<PBRTextureType, std::string> texturePaths, bool hasTexture)
{
    std::vector<VkDescriptorSetLayout> layouts(Engine::Settings::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(Engine::Settings::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    int textureCount = static_cast<int>(texture.getTextureCount());

    for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        if (isCube) {
            bufferInfo.buffer = texture.skyboxUniformResources[i]->buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(SkyboxUBO);
        }
        else {
            bufferInfo.buffer = texture.uniformResources[i]->buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
        }

        std::vector<VkDescriptorImageInfo> imageInfos(texturePaths.size());
        VkDescriptorImageInfo imageInfo{};
        
        if (hasTexture) {
            if (textureCount == -1) {
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                imageInfo.imageView = texture.textureResource->view;
                imageInfo.sampler = texture.textureResource->sampler;
                imageInfos.push_back(imageInfo);
            }
            else {
                int index = 0;
                for (auto& texturePair : texturePaths) {
                    PBRTextureType textureType = texturePair.first;
                    const std::string& texturePath = texturePair.second;

                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    imageInfo.imageView = texture.textureResources[index]->view;
                    imageInfo.sampler = texture.textureResources[index]->sampler;

                    imageInfos[index] = (imageInfo);
                    index++;
                }
            }
        }

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        VkWriteDescriptorSet uniformWrite{};
        uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformWrite.dstSet = descriptorSets[i];
        uniformWrite.dstBinding = 0;
        uniformWrite.dstArrayElement = 0;
        uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformWrite.descriptorCount = 1;
        uniformWrite.pBufferInfo = &bufferInfo;

        descriptorWrites.push_back(uniformWrite);

        if (hasTexture) {
            if (textureCount == -1) {
                VkWriteDescriptorSet imageWrite{};
                imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                imageWrite.dstSet = descriptorSets[i];
                imageWrite.dstBinding = 1;
                imageWrite.dstArrayElement = 0;
                imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageWrite.descriptorCount = 1;
                imageWrite.pImageInfo = &imageInfo;

                descriptorWrites.push_back(imageWrite);
            }
            else {
                for (uint32_t j = 0; j < imageInfos.size(); j++) {
                    VkWriteDescriptorSet imageWrite{};
                    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    imageWrite.dstSet = descriptorSets[i];
                    imageWrite.dstBinding = static_cast<uint32_t>(j + 1);
                    imageWrite.dstArrayElement = 0;
                    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    imageWrite.descriptorCount = 1;
                    imageWrite.pImageInfo = &imageInfos[j];

                    descriptorWrites.push_back(imageWrite);
                }
            }
        }
        
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
