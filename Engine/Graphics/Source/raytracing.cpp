#include "raytracing.h"
#include "frameBuffer.h"
#include "device.h"
#include "texture.h"
#include "commandBuffer.h"

void AccelerationStructure::create(Engine::Graphics::Device device, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(device.getDevice(), &bufferCreateInfo, nullptr, &buffer);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memoryRequirements);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device.getPhysicalDevice());
	vkAllocateMemory(device.getDevice(), &memoryAllocateInfo, nullptr, &memory);
	vkBindBufferMemory(device.getDevice(), buffer, memory, 0);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = buffer;
	accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = type;
	fpCreateAccelerationStructureKHR(device.getDevice(), &accelerationStructureCreateInfo, nullptr, &handle);

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = handle;
	deviceAddress = fpGetAccelerationStructureDeviceAddressKHR(device.getDevice(), &accelerationDeviceAddressInfo);
}

ScratchBuffer::ScratchBuffer(Engine::Graphics::Device device, VkDeviceSize size)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(device.getDevice(), &bufferCreateInfo, nullptr, &handle);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device.getDevice(), handle, &memoryRequirements);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device.getPhysicalDevice());
	vkAllocateMemory(device.getDevice(), &memoryAllocateInfo, nullptr, &memory);
	vkBindBufferMemory(device.getDevice(), handle, memory, 0);

	VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
	bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddressInfo.buffer = handle;
	deviceAddress = fpGetBufferDeviceAddressKHR(device.getDevice(), &bufferDeviceAddressInfo);
}

void StorageImage::create(Engine::Graphics::Device device, VkQueue queue, VkCommandPool commandPool, VkFormat format, VkExtent3D extent) 
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = extent;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkCreateImage(device.getDevice(), &imageInfo, nullptr, &image);

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(device.getDevice(), image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device.getPhysicalDevice());
	vkAllocateMemory(device.getDevice(), &memoryAllocateInfo, nullptr, &memory);
	vkBindImageMemory(device.getDevice(), image, memory, 0);

	VkImageViewCreateInfo colorImageView{};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = format;
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = image;
	vkCreateImageView(device.getDevice(), &colorImageView, nullptr, &view);

	if (commandPool == VK_NULL_HANDLE) {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = (uint32_t)device.getGraphicsQueue();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &commandPool);
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.getDevice(), &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(commandBuffer, &commandBufferInfo);

	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = 0;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0;

	VkFence fence;
	vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &fence);
	vkQueueSubmit(queue, 1, &submitInfo, fence);
	vkWaitForFences(device.getDevice(), 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(device.getDevice(), fence, nullptr);

	//vkFreeCommandBuffers(device.getDevice(), commandPool, 1, &commandBuffer);

}

void StorageImage::destroy(VkDevice device)
{
	if (view != VK_NULL_HANDLE) {
		vkDestroyImageView(device, view, nullptr);
		view = VK_NULL_HANDLE;
	}
	if (image != VK_NULL_HANDLE) {
		vkDestroyImage(device, image, nullptr);
		image = VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
}

VkDeviceAddress Engine::Graphics::Raytracing::getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAddress{};
	bufferDeviceAddress.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddress.buffer = buffer;
	return fpGetBufferDeviceAddressKHR(device, &bufferDeviceAddress);
}

std::vector<char> Engine::Graphics::Raytracing::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("failed to open file");

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule Engine::Graphics::Raytracing::createShaderModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module");

	return shaderModule;
}

void Engine::Graphics::Raytracing::initRaytracing(Engine::Graphics::Device device)
{
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR initRayTracingPipelineProperties{};
	initRayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkFormatProperties2 formatProps = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
	vkGetPhysicalDeviceFormatProperties2(device.getPhysicalDevice(), VK_FORMAT_R8G8B8A8_UNORM, &formatProps);

	if (!(formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
		VkFormat supportedFormats[] = { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB };

		for (auto format : supportedFormats) {
			vkGetPhysicalDeviceFormatProperties2(device.getPhysicalDevice(), format, &formatProps);
		}
	}

	VkPhysicalDeviceProperties2 deviceProperties{};
	deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties.pNext = &initRayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(device.getPhysicalDevice(), &deviceProperties);

	rayTracingPipelineProperties = initRayTracingPipelineProperties;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR initAccelerationStructureFeatures{};
	initAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &initAccelerationStructureFeatures;
	vkGetPhysicalDeviceFeatures2(device.getPhysicalDevice(), &deviceFeatures2);

	accelerationStructureFeatures = initAccelerationStructureFeatures;
}

auto Engine::Graphics::Raytracing::createBottomLevelAccelerationStructure(Engine::Graphics::Device device, uint32_t index)
{
	AccelerationStructure& blas = BLAS[index];

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = blas.handle;

	auto deviceAddress = fpGetAccelerationStructureDeviceAddressKHR(device.getDevice(), &accelerationDeviceAddressInfo);

	VkAccelerationStructureInstanceKHR blasInstance{};
	blasInstance.transform = Engine::Utility::convertMat4ToTransformMatrix(models[index].matrix);
	blasInstance.instanceCustomIndex = index;
	blasInstance.mask = 0xFF;
	blasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	blasInstance.accelerationStructureReference = deviceAddress;

	return blasInstance;
}

void Engine::Graphics::Raytracing::createBottomLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, RTScene model)
{
	VkBuffer transformMatrixBuffer;
	VkDeviceMemory transformMatrixBufferMemory;

	framebuffer.createBuffer(device, sizeof(glm::mat4), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, transformMatrixBuffer, transformMatrixBufferMemory, &model.matrix);
		
	uint32_t numTriangles = static_cast<uint32_t>(model.obj.i.size()) / 3;

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = getBufferDeviceAddress(device.getDevice(), model.obj.vb);
	accelerationStructureGeometry.geometry.triangles.maxVertex = model.obj.v.size();
	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = getBufferDeviceAddress(device.getDevice(), model.obj.ib);
	accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationStructureBuildGeometryInfo.pNext = nullptr;

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	fpGetAccelerationStructureBuildSizesKHR(device.getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelerationStructureBuildGeometryInfo, &numTriangles, &accelerationStructureBuildSizesInfo);

	AccelerationStructure blas;
	blas.create(device, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationStructureBuildSizesInfo);

	ScratchBuffer scratchBuffer(device, accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = blas.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	if (accelerationStructureFeatures.accelerationStructureHostCommands) {
		fpBuildAccelerationStructuresKHR(device.getDevice(), VK_NULL_HANDLE, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
	}
	else {
		VkCommandBuffer cmdbuf = commandBuffer.beginSingleTimeCommands(device.getDevice());
		fpCmdBuildAccelerationStructuresKHR(cmdbuf, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
		commandBuffer.endSingleTimeCommands(cmdbuf, device.getGraphicsQueue(), device.getDevice());
	}

	BLAS.push_back(blas);
}

void Engine::Graphics::Raytracing::createTopLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer)
{
	std::vector<VkAccelerationStructureInstanceKHR> blasInstances{};

	for (unsigned int i = 0; i < BLAS.size(); i++) {
		blasInstances.push_back(createBottomLevelAccelerationStructure(device, i));
	}

	framebuffer.createBuffer(device, sizeof(VkAccelerationStructureInstanceKHR) * blasInstances.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffer, instanceBufferMemory, blasInstances.data());

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(device.getDevice(), instanceBuffer);

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationStructureBuildGeometryInfo.pNext = nullptr;

	uint32_t primitiveCount = static_cast<uint32_t>(blasInstances.size());

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	
	fpGetAccelerationStructureBuildSizesKHR(device.getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelerationStructureBuildGeometryInfo, &primitiveCount, &accelerationStructureBuildSizesInfo);

	TLAS.create(device, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfo);

	if (TLAS.handle == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to create TLAS acceleration structures");
	}

	ScratchBuffer scratchBuffer(device, accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = TLAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = primitiveCount;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	if (accelerationStructureFeatures.accelerationStructureHostCommands) {
		fpBuildAccelerationStructuresKHR(device.getDevice(), VK_NULL_HANDLE, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
	}
	else {
		VkCommandBuffer cmdbuf = commandBuffer.beginSingleTimeCommands(device.getDevice());

		VkMemoryBarrier memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		vkCmdPipelineBarrier(
			cmdbuf,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			0,
			1, &memoryBarrier,
			0, nullptr,
			0, nullptr
		);

		fpCmdBuildAccelerationStructuresKHR(cmdbuf, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
		commandBuffer.endSingleTimeCommands(cmdbuf, device.getGraphicsQueue(), device.getDevice());
	}

	vkDestroyBuffer(device.getDevice(), scratchBuffer.handle, nullptr);
	vkFreeMemory(device.getDevice(), scratchBuffer.memory, nullptr);
}

void Engine::Graphics::Raytracing::updateTopLevelAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::FrameBuffer framebuffer, Engine::Graphics::CommandBuffer commandBuffer, bool rebuild)
{
	std::vector<VkAccelerationStructureInstanceKHR> blasInstances(BLAS.size());

	for (uint32_t i = 0; i < BLAS.size(); ++i) {
		VkAccelerationStructureDeviceAddressInfoKHR addrInfo{};
		addrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		addrInfo.accelerationStructure = BLAS[i].handle;
		uint64_t deviceAddress = fpGetAccelerationStructureDeviceAddressKHR(device.getDevice(), &addrInfo);

		blasInstances[i].transform = Engine::Utility::convertMat4ToTransformMatrix(models[i].matrix);
		blasInstances[i].instanceCustomIndex = i;
		blasInstances[i].mask = 0xFF;
		blasInstances[i].instanceShaderBindingTableRecordOffset = 0;
		blasInstances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		blasInstances[i].accelerationStructureReference = deviceAddress;
	}

	void* data;
	vkMapMemory(device.getDevice(), instanceBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
	memcpy(data, blasInstances.data(), blasInstances.size() * sizeof(VkAccelerationStructureInstanceKHR));
	vkUnmapMemory(device.getDevice(), instanceBufferMemory);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = instanceBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 8;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(device.getDevice(), 1, &descriptorWrite, 0, nullptr);

	VkMappedMemoryRange memoryRange{};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = instanceBufferMemory;
	memoryRange.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(device.getDevice(), 1, &memoryRange);

	vkUnmapMemory(device.getDevice(), instanceBufferMemory);

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(device.getDevice(), instanceBuffer);

	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data = instanceDataDeviceAddress;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	buildInfo.srcAccelerationStructure = TLAS.handle;
	buildInfo.dstAccelerationStructure = TLAS.handle;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometry;

	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
	buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	uint32_t primitiveCount = static_cast<uint32_t>(blasInstances.size());

	fpGetAccelerationStructureBuildSizesKHR(
		device.getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&primitiveCount,
		&buildSizesInfo
	);

	ScratchBuffer scratchBuffer(device, rebuild ? buildSizesInfo.buildScratchSize : buildSizesInfo.updateScratchSize);
	buildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	rangeInfo.primitiveCount = static_cast<uint32_t>(blasInstances.size());

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos = { &rangeInfo };

	VkCommandBuffer cmdbuf = commandBuffer.beginSingleTimeCommands(device.getDevice());
	VkMemoryBarrier hostWriteBarrier{};
	hostWriteBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	hostWriteBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	hostWriteBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

	vkCmdPipelineBarrier(
		cmdbuf,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		0,
		1, &hostWriteBarrier,
		0, nullptr,
		0, nullptr
	);
	
	fpCmdBuildAccelerationStructuresKHR(cmdbuf, 1, &buildInfo, rangeInfos.data());
	
	VkMemoryBarrier asBarrier{};
	asBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	asBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	asBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
		VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		cmdbuf,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		1, &asBarrier,
		0, nullptr,
		0, nullptr
	);

	commandBuffer.endSingleTimeCommands(cmdbuf, device.getGraphicsQueue(), device.getDevice());

	VkWriteDescriptorSetAccelerationStructureKHR asWrite{};
	asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	asWrite.accelerationStructureCount = 1;
	asWrite.pAccelerationStructures = &TLAS.handle;

	VkWriteDescriptorSet tlasDescriptorWrite{};
	tlasDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tlasDescriptorWrite.pNext = &asWrite;
	tlasDescriptorWrite.dstSet = descriptorSet;
	tlasDescriptorWrite.dstBinding = 0;
	tlasDescriptorWrite.descriptorCount = 1;
	tlasDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

	vkUpdateDescriptorSets(device.getDevice(), 1, &tlasDescriptorWrite, 0, nullptr);

	vkDestroyBuffer(device.getDevice(), scratchBuffer.handle, nullptr);
	vkFreeMemory(device.getDevice(), scratchBuffer.memory, nullptr);
}

void Engine::Graphics::Raytracing::buildAccelerationStructure(Engine::Graphics::Device device, Engine::Graphics::CommandBuffer commandbuffer, Engine::Graphics::FrameBuffer framebuffer)
{
	VkCommandBuffer commandBuffer = commandbuffer.beginSingleTimeCommands(device.getDevice());

	for (auto& model : models) {
		createBottomLevelAccelerationStructure(device, framebuffer, commandbuffer, model);
	}

	createTopLevelAccelerationStructure(device, framebuffer, commandbuffer);
	commandbuffer.endSingleTimeCommands(commandBuffer, device.getGraphicsQueue(), device.getDevice());
}

void Engine::Graphics::Raytracing::createShaderBindingTables(Engine::Graphics::Device device)
{
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
	const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	fpGetRayTracingShaderGroupHandlesKHR(device.getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());

	auto createSBTBuffer = [&](VkBuffer& buffer, VkDeviceMemory& memory, const void* data, VkDeviceSize size) {
		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.size = size;

		vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer);

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memReqs);

		VkMemoryAllocateFlagsInfo allocFlagsInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
		allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

		VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = Engine::Utility::findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, device.getPhysicalDevice());
		allocInfo.pNext = &allocFlagsInfo;

		vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &memory);
		vkBindBufferMemory(device.getDevice(), buffer, memory, 0);

		void* mapped;
		vkMapMemory(device.getDevice(), memory, 0, size, 0, &mapped);
		memcpy(mapped, data, size);
		vkUnmapMemory(device.getDevice(), memory);
		};

	createSBTBuffer(raygenSBTBuffer, raygenSBTMemory, shaderHandleStorage.data(), handleSizeAligned);
	createSBTBuffer(missSBTBuffer, missSBTMemory, shaderHandleStorage.data() + handleSizeAligned, handleSizeAligned);
	createSBTBuffer(hitSBTBuffer, hitSBTMemory, shaderHandleStorage.data() + handleSizeAligned * 2, handleSizeAligned);
}

void Engine::Graphics::Raytracing::createDescriptorSets(Engine::Graphics::Device device, Engine::Graphics::Texture skyboxTexture)
{
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 * static_cast<uint32_t>(models.size())},
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.maxSets = 1;
	
	vkCreateDescriptorPool(device.getDevice(), &descriptorPoolCreateInfo, nullptr, &descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	vkAllocateDescriptorSets(device.getDevice(), &descriptorSetAllocateInfo, &descriptorSet);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
	descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	descriptorAccelerationStructureInfo.pAccelerationStructures = &TLAS.handle;

	VkWriteDescriptorSet accelerationStructureWrite{};
	accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
	accelerationStructureWrite.dstSet = descriptorSet;
	accelerationStructureWrite.dstBinding = 0;
	accelerationStructureWrite.descriptorCount = 1;
	accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	writeDescriptorSets.push_back(accelerationStructureWrite);

	VkDescriptorImageInfo storageImageInfo{};
	storageImageInfo.imageView = storageImage.view;
	storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorImageInfo accumImageInfo{};
	accumImageInfo.imageView = accumulationImage.view;
	accumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet storageImageWrite{};
	storageImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	storageImageWrite.dstSet = descriptorSet;
	storageImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	storageImageWrite.dstBinding = 1;
	storageImageWrite.pImageInfo = &storageImageInfo;
	storageImageWrite.descriptorCount = 1;
	writeDescriptorSets.push_back(storageImageWrite);

	VkWriteDescriptorSet accumImageWrite{};
	accumImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accumImageWrite.dstSet = descriptorSet;
	accumImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	accumImageWrite.dstBinding = 2;
	accumImageWrite.pImageInfo = &accumImageInfo;
	accumImageWrite.descriptorCount = 1;
	writeDescriptorSets.push_back(accumImageWrite);

	VkDescriptorBufferInfo uboInfo{};
	uboInfo.buffer = uniformBuffer;
	uboInfo.offset = 0;
	uboInfo.range = sizeof(RaytracingUniformBufferObject);

	VkWriteDescriptorSet uboWrite{};
	uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboWrite.dstSet = descriptorSet;
	uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboWrite.dstBinding = 3;
	uboWrite.pBufferInfo = &uboInfo;
	uboWrite.descriptorCount = 1;
	writeDescriptorSets.push_back(uboWrite);

	std::vector<VkDescriptorBufferInfo> vBufferInfos;
	for (auto& model : models) {
		if (model.obj.vb != VK_NULL_HANDLE) {
			vBufferInfos.push_back({ model.obj.vb, 0, VK_WHOLE_SIZE });
		}
	}

	if (!vBufferInfos.empty()) {
		VkWriteDescriptorSet vBufferWrite{};
		vBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vBufferWrite.dstSet = descriptorSet;
		vBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vBufferWrite.dstBinding = 4;
		vBufferWrite.descriptorCount = static_cast<uint32_t>(vBufferInfos.size());
		vBufferWrite.pBufferInfo = vBufferInfos.data();
		writeDescriptorSets.push_back(vBufferWrite);
	}

	std::vector<VkDescriptorBufferInfo> iBufferInfos;
	for (auto& model : models) {
		if (model.obj.ib != VK_NULL_HANDLE) {
			iBufferInfos.push_back({ model.obj.ib, 0, VK_WHOLE_SIZE });
		}
	}

	if (!iBufferInfos.empty()) {
		VkWriteDescriptorSet iBufferWrite{};
		iBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		iBufferWrite.dstSet = descriptorSet;
		iBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		iBufferWrite.dstBinding = 5;
		iBufferWrite.descriptorCount = static_cast<uint32_t>(iBufferInfos.size());
		iBufferWrite.pBufferInfo = iBufferInfos.data();
		writeDescriptorSets.push_back(iBufferWrite);
	}

	VkDescriptorImageInfo skyboxInfo{};
	skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	skyboxInfo.imageView = skyboxTexture.getTextureImageView();
	skyboxInfo.sampler = skyboxTexture.getTextureSampler();

	VkWriteDescriptorSet skyboxWrite{};
	skyboxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	skyboxWrite.dstSet = descriptorSet;
	skyboxWrite.dstBinding = 6;
	skyboxWrite.dstArrayElement = 0;
	skyboxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	skyboxWrite.descriptorCount = 1;
	skyboxWrite.pImageInfo = &skyboxInfo;
	writeDescriptorSets.push_back(skyboxWrite);

	std::vector<VkDescriptorBufferInfo> mBufferInfos;
	for (auto& model : models) {
		if (model.obj.mb != VK_NULL_HANDLE) {
			mBufferInfos.push_back({ model.obj.mb, 0, VK_WHOLE_SIZE });
		}
	}

	if (!mBufferInfos.empty()) {
		VkWriteDescriptorSet mBufferWrite{};
		mBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mBufferWrite.dstSet = descriptorSet;
		mBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		mBufferWrite.dstBinding = 7;
		mBufferWrite.descriptorCount = static_cast<uint32_t>(mBufferInfos.size());
		mBufferWrite.pBufferInfo = mBufferInfos.data();
		writeDescriptorSets.push_back(mBufferWrite);
	}

	VkDescriptorBufferInfo instanceTransformInfo{};
	instanceTransformInfo.buffer = instanceBuffer;
	instanceTransformInfo.offset = 0;
	instanceTransformInfo.range = sizeof(glm::mat4) * models.size();

	VkWriteDescriptorSet instanceTransformWrite{};
	instanceTransformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	instanceTransformWrite.dstSet = descriptorSet;
	instanceTransformWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	instanceTransformWrite.dstBinding = 8;
	instanceTransformWrite.descriptorCount = 1;
	instanceTransformWrite.pBufferInfo = &instanceTransformInfo;

	writeDescriptorSets.push_back(instanceTransformWrite);

	vkUpdateDescriptorSets(
		device.getDevice(),
		static_cast<uint32_t>(writeDescriptorSets.size()),
		writeDescriptorSets.data(),
		0,
		nullptr
	);
}

void Engine::Graphics::Raytracing::updateDescriptorSets(Engine::Graphics::Device device)
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkDescriptorImageInfo storageImageInfo{};
	storageImageInfo.imageView = storageImage.view;
	storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkDescriptorImageInfo accumImageInfo{};
	accumImageInfo.imageView = accumulationImage.view;
	accumImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet storageImageWrite{};
	storageImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	storageImageWrite.dstSet = descriptorSet;
	storageImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	storageImageWrite.dstBinding = 1;
	storageImageWrite.pImageInfo = &storageImageInfo;
	storageImageWrite.descriptorCount = 1;
	writeDescriptorSets.push_back(storageImageWrite);

	VkWriteDescriptorSet accumImageWrite{};
	accumImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	accumImageWrite.dstSet = descriptorSet;
	accumImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	accumImageWrite.dstBinding = 2;
	accumImageWrite.pImageInfo = &accumImageInfo;
	accumImageWrite.descriptorCount = 1;
	writeDescriptorSets.push_back(accumImageWrite);

	vkUpdateDescriptorSets(
		device.getDevice(), 
		static_cast<uint32_t>(writeDescriptorSets.size()), 
		writeDescriptorSets.data(), 
		0, 
		nullptr
	);

}

void Engine::Graphics::Raytracing::createRayTracingPipeline(Engine::Graphics::Device device, std::string raygenShaderPath, std::string missShaderPath, std::string chitShaderPath)
{
	auto raygenShaderCode = readFile(raygenShaderPath);
	auto missShaderCode = readFile(missShaderPath);
	auto closestHitShaderCode = readFile(chitShaderPath);
	
	VkShaderModule raygenShaderModule = createShaderModule(device.getDevice(), raygenShaderCode);
	VkShaderModule missShaderModule = createShaderModule(device.getDevice(), missShaderCode);
	VkShaderModule closestHitShaderModule = createShaderModule(device.getDevice(), closestHitShaderCode);

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr},
		{1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
		{2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
		{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, nullptr},
		{4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(models.size()), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, nullptr},
		{5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(models.size()), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, nullptr},
		{6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr },
		{7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(models.size()), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR, nullptr},
		{8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr},
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
	vkCreateDescriptorSetLayout(device.getDevice(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderGroups.clear();

	shaderStages.push_back({ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, raygenShaderModule, "main", nullptr });
	shaderGroups.push_back({ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR, nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, static_cast<uint32_t>(shaderStages.size()) - 1, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, nullptr });

	shaderStages.push_back({
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr,
		0,
		VK_SHADER_STAGE_MISS_BIT_KHR,
		missShaderModule,
		"main",
		nullptr
		});
	shaderGroups.push_back({
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		nullptr,
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		static_cast<uint32_t>(shaderStages.size()) - 1,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		nullptr
		});
	
	shaderStages.push_back({
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr,
		0,
		VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
		closestHitShaderModule,
		"main",
		nullptr
		});

	VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
	hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	hitGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
	hitGroup.generalShader = VK_SHADER_UNUSED_KHR;
	hitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	hitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(hitGroup);

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo{};
	rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineCreateInfo.pStages = shaderStages.data();
	rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
	rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 31;
	rayTracingPipelineCreateInfo.layout = pipelineLayout;

	fpCreateRayTracingPipelinesKHR(device.getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &pipeline);

	vkDestroyShaderModule(device.getDevice(), raygenShaderModule, nullptr);
	vkDestroyShaderModule(device.getDevice(), missShaderModule, nullptr);
	vkDestroyShaderModule(device.getDevice(), closestHitShaderModule, nullptr);
}

void Engine::Graphics::Raytracing::createImage(Engine::Graphics::Device device, VkCommandPool commandPool, VkExtent2D extent)
{
	storageImage.create(device, device.getGraphicsQueue(), commandPool, VK_FORMAT_R8G8B8A8_UNORM, { extent.width, extent.height, 1 });
	accumulationImage.create(device, device.getGraphicsQueue(), commandPool, VK_FORMAT_R32G32B32A32_SFLOAT, { extent.width, extent.height, 1 });
}

void Engine::Graphics::Raytracing::traceRays(VkDevice device, VkCommandBuffer commandBuffer, VkExtent2D extent, VkImage swapchainImage, uint32_t currentImageIndex)
{
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = (handleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) &
		~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);

	VkStridedDeviceAddressRegionKHR raygenSBTRegion{};
	raygenSBTRegion.deviceAddress = getBufferDeviceAddress(device, raygenSBTBuffer);
	raygenSBTRegion.stride = handleSizeAligned;
	raygenSBTRegion.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR missSBTRegion{};
	missSBTRegion.deviceAddress = getBufferDeviceAddress(device, missSBTBuffer);
	missSBTRegion.stride = handleSizeAligned;
	missSBTRegion.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR hitSBTRegion{};
	hitSBTRegion.deviceAddress = getBufferDeviceAddress(device, hitSBTBuffer);
	hitSBTRegion.stride = handleSizeAligned;
	hitSBTRegion.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR callableSBTRegion{};

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout,
		0, 1, &descriptorSet, 0, nullptr);

	fpCmdTraceRaysKHR(
		commandBuffer,
		&raygenSBTRegion,
		&missSBTRegion,
		&hitSBTRegion,
		&callableSBTRegion,
		extent.width,
		extent.height,
		1
	);

	VkImageSubresourceRange subresourceRange = {
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1, 0, 1
	};

	VkImageMemoryBarrier storageBarrier = {};
	storageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	storageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	storageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	storageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	storageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	storageBarrier.image = storageImage.image;
	storageBarrier.subresourceRange = subresourceRange;

	VkImageMemoryBarrier accumBarrier{};
	accumBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	accumBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	accumBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	accumBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	accumBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	accumBarrier.image = accumulationImage.image;
	accumBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &storageBarrier
	);

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &accumBarrier
	);

	VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { extent.width, extent.height, 1 };

	vkCmdCopyImage(
		commandBuffer,
		storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		accumulationImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copyRegion
	);

	VkImageMemoryBarrier accumGeneral{};
	accumGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	accumGeneral.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	accumGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	accumGeneral.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	accumGeneral.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	accumGeneral.image = accumulationImage.image;
	accumGeneral.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		0,
		0, nullptr,
		0, nullptr,
		1, &accumGeneral
	);

	VkImageMemoryBarrier swapchainBarrier = {};
	swapchainBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	swapchainBarrier.srcAccessMask = 0;
	swapchainBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	swapchainBarrier.image = swapchainImage;
	swapchainBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &swapchainBarrier
	);

	VkImageBlit blitRegion{};
	blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	blitRegion.srcOffsets[0] = { 0, 0, 0 };
	blitRegion.srcOffsets[1] = { (int)extent.width, (int)extent.height, 1 };
	blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	blitRegion.dstOffsets[0] = { 0, 0, 0 };
	blitRegion.dstOffsets[1] = { (int)extent.width, (int)extent.height, 1 };

	vkCmdBlitImage(
		commandBuffer,
		accumulationImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blitRegion,
		VK_FILTER_LINEAR
	);
	
	VkImageMemoryBarrier presentBarrier = {};
	presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	presentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	presentBarrier.dstAccessMask = 0;
	presentBarrier.image = swapchainImage;
	presentBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &presentBarrier
	);
	
}

void Engine::Graphics::Raytracing::createUniformBuffer(Engine::Graphics::Device device)
{
	VkDeviceSize bufferSize = sizeof(RaytracingUniformBufferObject);

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &uniformBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create uniform buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getDevice(), uniformBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Engine::Utility::findMemoryType(
		memRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		device.getPhysicalDevice()
	);

	if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &uniformBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate uniform buffer memory!");
	}

	vkBindBufferMemory(device.getDevice(), uniformBuffer, uniformBufferMemory, 0);
	vkMapMemory(device.getDevice(), uniformBufferMemory, 0, bufferSize, 0, &uboMapped);
}

void Engine::Graphics::Raytracing::updateUBO(Engine::Graphics::Device device)
{
	memcpy(uboMapped, &uboData, sizeof(uboData));
}

void Engine::Graphics::Raytracing::cleanup(VkDevice device)
{
	vkDestroyBuffer(device, uniformBuffer, nullptr);
	vkFreeMemory(device, uniformBufferMemory, nullptr);

	vkDestroyBuffer(device, raygenSBTBuffer, nullptr);
	vkFreeMemory(device, raygenSBTMemory, nullptr);
	vkDestroyBuffer(device, missSBTBuffer, nullptr);
	vkFreeMemory(device, missSBTMemory, nullptr);
	vkDestroyBuffer(device, hitSBTBuffer, nullptr);
	vkFreeMemory(device, hitSBTMemory, nullptr);

	fpDestroyAccelerationStructureKHR(device, TLAS.handle, nullptr);
	vkDestroyBuffer(device, TLAS.buffer, nullptr);
	vkFreeMemory(device, TLAS.memory, nullptr);

	for (auto& blas : BLAS) {
		fpDestroyAccelerationStructureKHR(device, blas.handle, nullptr);
		vkDestroyBuffer(device, blas.buffer, nullptr);
		vkFreeMemory(device, blas.memory, nullptr);
	}

	storageImage.destroy(device);
	accumulationImage.destroy(device);

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}
