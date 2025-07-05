#include "core.h"

void Engine::Core::Application::run()
{
	initWindow();
	initVulkan();
	initImGui();
	mainLoop();
	cleanup();
}

void Engine::Core::Application::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	char title[256];
	title[255] = '\0';

	std::snprintf(title, 255, "%s", "Fortify");

	window = glfwCreateWindow(Engine::Settings::WIDTH, Engine::Settings::HEIGHT, title, nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void Engine::Core::Application::initVulkan()
{
	instance.createInstance();
	instance.setupDebugMessenger();
	instance.createSurface(window);
	device.pickPhysicalDevice(instance);
	sampler.setSamples(device.getPhysicalDevice());
	device.createLogicalDevice(instance.getSurface());

	fpCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCreateAccelerationStructureKHR"));
	fpDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkDestroyAccelerationStructureKHR"));
	fpGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
	fpCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdBuildAccelerationStructuresKHR"));
	fpGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
	fpCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCreateRayTracingPipelinesKHR"));
	fpGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
	fpCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdTraceRaysKHR"));
	fpCmdTraceRaysIndirectKHR = reinterpret_cast<PFN_vkCmdTraceRaysIndirectKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdTraceRaysIndirectKHR"));
	fpCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdBeginDebugUtilsLabelEXT"));
	fpCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdEndDebugUtilsLabelEXT"));
	fpGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetBufferDeviceAddressKHR"));
	fpBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkBuildAccelerationStructureKHR"));

	swapchain.createSwapChain(window, instance, device);
	swapchain.createImageViews(device.getDevice());

	VkExtent3D storageImageExtent = {
		swapchain.getSwapchainExtent().width,
		swapchain.getSwapchainExtent().height,
		1
	};

	renderpass.createRenderPass(device, sampler.getSamples(), swapchain);
	renderpass.setupLayoutBindings(device.getDevice());

	commandbuffer.createCommandPool(device, instance.getSurface());
	framebuffer.createColorResources(device, swapchain, sampler.getSamples());
	framebuffer.createDepthResources(device, swapchain, sampler.getSamples(), commandbuffer);
	framebuffer.createFramebuffers(device.getDevice(), swapchain, renderpass.getRenderPass());
	commandbuffer.createCommandBuffers(device.getDevice());

	raytrace.initRaytracing(device);

	raytrace.storageImage.create(device, device.getGraphicsQueue(), commandbuffer.getCommandPool(), VK_FORMAT_R8G8B8A8_UNORM, storageImageExtent);
	raytrace.accumulationImage.create(device, device.getGraphicsQueue(), commandbuffer.getCommandPool(), VK_FORMAT_R8G8B8A8_UNORM, storageImageExtent);

	rtscenemanager.add("textures/backpack/backpack.obj");
	rtscenemanager.add("textures/viking_room/viking_room.obj");
	rtscenemanager.add("textures/backpack/backpack.obj");

	rtscenemanager.pushToAccelerationStructure(raytrace.models);

	raytrace.buildAccelerationStructure(device, commandbuffer, framebuffer);

	std::vector<std::string> skyboxPaths = {
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg"
	};

	Engine::Graphics::Texture skyboxTexture;
	skyboxTexture.createCubemap(skyboxPaths, device, commandbuffer, framebuffer, sampler, false);
	skyboxTexture.createTextureImageView(swapchain, device.getDevice(), true);
	skyboxTexture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), true);

	std::unordered_map<PBRTextureType, std::string> pbrTextures = {
		{ PBRTextureType::Albedo, "textures/backpack/diffuse.jpg" },
		{ PBRTextureType::Normal, "textures/backpack/normal.png" },
		{ PBRTextureType::AmbientOcclusion, "textures/backpack/ao.jpg" },
		{ PBRTextureType::Roughness, "textures/backpack/roughness.jpg" },
		{ PBRTextureType::Specular, "textures/backpack/specular.jpg" },
	};

	scenemanager.addEntity<CubeVertex, EntityType::Skybox>("shaders/spv/skyboxVert.spv", "shaders/spv/skyboxFrag.spv", skyboxPaths, "", true);
	//scenemanager.addEntity<Vertex, EntityType::Object>("shaders/spv/vert.spv", "shaders/spv/frag.spv", "textures/viking_room/viking_room.png", "textures/viking_room/viking_room.obj", false);
	//scenemanager.addEntity<Vertex, EntityType::PBRObject>("shaders/spv/textureMapVert.spv", "shaders/spv/textureMapFrag.spv", pbrTextures, "textures/backpack/backpack.obj", false);
	//scenemanager.addEntity<Vertex, EntityType::MatObject>("shaders/spv/textureMapVert.spv", "shaders/spv/textureMapFrag.spv", "textures/backpack/backpack.mtl", "textures/backpack/backpack.obj", true);
	scenemanager.addEntity<Vertex, EntityType::Light>("shaders/spv/lightVert.spv", "shaders/spv/lightFrag.spv", "", "", false);
	//scenemanager.addEntity<Vertex, EntityType::Primitive>("shaders/spv/primitiveVert.spv", "shaders/spv/primitiveFrag.spv", PrimitiveType::Plane, "", false);
	
	raytrace.createRayTracingPipeline(device, "shaders/spv/raytraceRaygen.spv", "shaders/spv/raytraceMiss.spv", "shaders/spv/raytraceChit.spv");
	raytrace.createShaderBindingTables(device);
	raytrace.createUniformBuffer(device);
	raytrace.createDescriptorSets(device, skyboxTexture);

	texture.createSyncObjects(device.getDevice());
}

void Engine::Core::Application::initImGui()
{
	createImGuiRenderPass();

	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 10000;
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(device.getDevice(), &poolInfo, nullptr, &imguiPool)) {
		throw std::runtime_error("failed to create imgui descriptor pool");
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = instance.getInstance();
	initInfo.PhysicalDevice = device.getPhysicalDevice();
	initInfo.Device = device.getDevice();
	initInfo.Queue = device.getGraphicsQueue();
	initInfo.DescriptorPool = imguiPool;
	initInfo.RenderPass = imguiRenderPass;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = 3;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&initInfo);

	VkCommandBuffer imguiCB = commandbuffer.beginSingleTimeCommands(device.getDevice());
	ImGui_ImplVulkan_CreateFontsTexture();
	commandbuffer.endSingleTimeCommands(imguiCB, device.getGraphicsQueue(), device.getDevice());
	ImGui_ImplVulkan_DestroyFontsTexture();

	createImGuiFramebuffers();
}

void Engine::Core::Application::mainLoop()
{
	float prev = static_cast<float>(glfwGetTime());
	float prevFPS = static_cast<float>(glfwGetTime());
	int frameCount = 0;
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	const GLFWvidmode* currMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	Entity entity;
	bool selectVertex = false;
	bool selectFragment = false;
	bool selectTexture = false;
	bool selectModel = false;
	bool selectMat = false;
	bool useRaytracer = false;
	bool setShaderPath = false;
	bool toggleVsync = true;
	bool fullscreenTrigger = false;
	int currRefreshRate = currMode->refreshRate;

	const char* shaderPath = "";
	std::vector<const char*> shaderPaths;

	ImGui::FileBrowser file;
	file.SetTitle("File Browser");

	while (!glfwWindowShouldClose(window)) {
		float curr = static_cast<float>(glfwGetTime());
		float currFPS = static_cast<float>(glfwGetTime());
		deltaTime = curr - prev;
		fpsCounter = currFPS - prevFPS;
		prev = curr;
		frameCount++;

		float msPerFrame = deltaTime * 1000.0f;

		processInput(window);

		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (!isFocused) {
			io.WantCaptureMouse = true;
			io.WantCaptureKeyboard = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			io.WantCaptureKeyboard = false;
			io.WantCaptureMouse = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		}

		ImGui::Begin("Fortify");

		if (ImGui::BeginTabBar("Settings")) {
			if (ImGui::BeginTabItem("General")) {
				ImGui::Text("%.3f ms/frame (%.1f FPS)", msPerFrame, io.Framerate);
				
				if(ImGui::Checkbox("Fullscreen", &fullscreenTrigger)) {
					fullscreen = !fullscreen;

					if (fullscreen) {
						const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
						glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
					}
					else {
						glfwSetWindowMonitor(window, nullptr, 100, 100, Engine::Settings::WIDTH, Engine::Settings::HEIGHT, 0);
					}

					framebufferResized = true;
				}

				if (ImGui::Checkbox("VSync", &toggleVsync)) {
					recreateSwapchainFlag = !recreateSwapchainFlag;
				}

				ImGui::Checkbox("Enable Raytracing", &useRaytracer);

				ImGui::EndTabItem();
			}

			if (useRaytracer) {
				ImGui::Checkbox("Accumulate Frames", &accumulateFrames);

				if (accumulateFrames) {
					ImGui::SameLine();
					ImGui::Text("%d", raytrace.uboData.sampleCount);
				}
			}
			ImGui::EndTabBar();
		}

		ImGui::NewLine();

		if (useRaytracer) {
			rtscenemanager.updateScene();
		}
		else {
			/*
			if (ImGui::Button("Set Shader Path")) {
				setShaderPath = true;
				file.Open();
			}

			if (setShaderPath) {
				file.Display();
				if (file.HasSelected()) {
					std::string strPath = file.GetDirectory().string();
					shaderPath = strPath.c_str();
					std::cout << shaderPath << std::endl;
					file.ClearSelected();
					shaderPaths = Engine::Utility::getShaderPaths(shaderPath);
					scenemanager.setShaderPaths(shaderPaths);
					setShaderPath = false;
				}
			}*/

			scenemanager.updateScene();

			if (ImGui::Button("Add Entity")) {
				entity.add = true;
			}

			if (entity.add) {
				ImGui::OpenPopup("Add Entity");

				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal("Add Entity", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::SetItemDefaultFocus();

					const char* entityItems[] = {
						scenemanager.entityString(EntityType::Object),
						scenemanager.entityString(EntityType::Skybox),
						scenemanager.entityString(EntityType::UI),
						scenemanager.entityString(EntityType::Light),
						scenemanager.entityString(EntityType::Terrain),
						scenemanager.entityString(EntityType::Particle),
						scenemanager.entityString(EntityType::PBRObject),
						scenemanager.entityString(EntityType::MatObject),
						scenemanager.entityString(EntityType::Primitive)
					};

					int currentItem = static_cast<int>(entity.type);

					if (ImGui::Combo("Entity Type", &currentItem, entityItems, IM_ARRAYSIZE(entityItems))) {
						entity.type = static_cast<EntityType>(currentItem);
					}

					if (ImGui::Button("Vertex Shader")) {
						selectVertex = true;
						file.Open();
					}

					if (selectVertex) {
						file.Display();
						if (file.HasSelected()) {
							entity.vertexPath = file.GetSelected().string();
							file.ClearSelected();
							selectVertex = false;
						}
					}

					ImGui::SameLine();

					ImGui::Text("%s", entity.vertexPath.c_str());

					if (ImGui::Button("Fragment Shader")) {
						selectFragment = true;
						file.Open();
					}

					if (selectFragment) {
						file.Display();
						if (file.HasSelected()) {
							entity.fragmentPath = file.GetSelected().string();
							file.ClearSelected();
							selectFragment = false;
						}
					}

					ImGui::SameLine();

					ImGui::Text("%s", entity.fragmentPath.c_str());

					if (entity.type == EntityType::Object) {
						if (ImGui::Button("Texture Path")) {
							selectTexture = true;
							file.Open();
						}

						if (selectTexture) {
							file.Display();
							if (file.HasSelected()) {
								entity.texturePath = file.GetSelected().string();
								file.ClearSelected();
								selectTexture = false;
							}
						}

						ImGui::SameLine();
						ImGui::Text("%s", entity.texturePath.c_str());

						if (ImGui::Button("Model Path")) {
							selectModel = true;
							file.Open();
						}

						if (selectModel) {
							file.Display();
							if (file.HasSelected()) {
								entity.modelPath = file.GetSelected().string();
								file.ClearSelected();
								selectModel = false;
							}
						}

						ImGui::SameLine();

						ImGui::Text("%s", entity.modelPath.c_str());

						ImGui::Checkbox("Flip Texture", &entity.flipTexture);
					}
					else if (entity.type == EntityType::PBRObject) {
						const char* textureType[] = {
							scenemanager.textureString(PBRTextureType::Albedo),
							scenemanager.textureString(PBRTextureType::Normal),
							scenemanager.textureString(PBRTextureType::Roughness),
							scenemanager.textureString(PBRTextureType::Metalness),
							scenemanager.textureString(PBRTextureType::AmbientOcclusion),
							scenemanager.textureString(PBRTextureType::Specular),
						};

						int currentItem = static_cast<int>(entity.textureType) - 1;

						if (ImGui::Combo("Texture Type", &currentItem, textureType, IM_ARRAYSIZE(textureType))) {
							entity.textureType = static_cast<PBRTextureType>(currentItem + 1);
						}

						if (ImGui::Button("Add Texture")) {
							selectTexture = true;
							file.Open();
						}

						if (selectTexture) {
							file.Display();
							if (file.HasSelected()) {
								entity.texturePaths.insert({ entity.textureType , file.GetSelected().string() });
								file.ClearSelected();
								selectTexture = false;
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Remove Texture")) {
							entity.texturePaths.erase(static_cast<PBRTextureType>(currentItem + 1));
						}

						for (auto& texturePair : entity.texturePaths) {
							ImGui::Text("%s %s", scenemanager.textureString(texturePair.first), texturePair.second.c_str());
						}

						if (ImGui::Button("Model Path")) {
							selectModel = true;
							file.Open();
						}

						if (selectModel) {
							file.Display();
							if (file.HasSelected()) {
								entity.modelPath = file.GetSelected().string();
								file.ClearSelected();
								selectModel = false;
							}
						}

						ImGui::SameLine();

						ImGui::Text("%s", entity.modelPath.c_str());

						ImGui::Checkbox("Flip Texture", &entity.flipTexture);
					}
					else if (entity.type == EntityType::Skybox) {
						if (!scenemanager.hasSkybox()) {
							ImGui::Text("Skybox paths include order matters!");

							if (ImGui::Button("Add Texture")) {
								selectTexture = true;
								file.Open();
							}

							if (selectTexture) {
								file.Display();
								if (file.HasSelected()) {
									entity.skyboxPaths.push_back(file.GetSelected().string());
									file.ClearSelected();
									selectTexture = false;
								}
							}

							ImGui::SameLine();

							if (ImGui::Button("Remove Texture")) {
								entity.texturePaths.erase(static_cast<PBRTextureType>(currentItem + 1));
							}

							for (auto& skybox : entity.skyboxPaths) {
								ImGui::Text("%s", skybox.c_str());
							}
						}

						ImGui::Checkbox("Flip Texture", &entity.flipTexture);
					}
					else if (entity.type == EntityType::MatObject) {
						if (ImGui::Button("Model Path")) {
							selectModel = true;
							file.Open();
						}

						if (selectModel) {
							file.Display();
							if (file.HasSelected()) {
								entity.modelPath = file.GetSelected().string();
								file.ClearSelected();
								selectModel = false;
							}
						}

						ImGui::SameLine();

						ImGui::Text("%s", entity.modelPath.c_str());

						if (ImGui::Button("Material Path")) {
							selectMat = true;
							file.Open();
						}

						if (selectMat) {
							file.Display();
							if (file.HasSelected()) {
								entity.materialPath = file.GetSelected().string();
								file.ClearSelected();
								selectMat = false;
							}
						}

						ImGui::SameLine();

						ImGui::Text("%s", entity.materialPath.c_str());

						ImGui::Checkbox("Flip Texture", &entity.flipTexture);
					}
					else if (entity.type == EntityType::Primitive) {
						const char* primitiveType[] = {
							scenemanager.primitiveString(PrimitiveType::Cube),
							scenemanager.primitiveString(PrimitiveType::Sphere),
							scenemanager.primitiveString(PrimitiveType::Plane),
						};

						int currentItem = static_cast<int>(entity.primitiveType);

						if (ImGui::Combo("Primitive Type", &currentItem, primitiveType, IM_ARRAYSIZE(primitiveType))) {
							entity.primitiveType = static_cast<PrimitiveType>(currentItem);
						}
					}

					if (ImGui::Button("Close")) {
						ImGui::CloseCurrentPopup();
						entity.add = false;
					}

					ImGui::SameLine();

					if (ImGui::Button("Add")) {
						ImGui::CloseCurrentPopup();
						EntityType type = static_cast<EntityType>(entity.type);

						switch (type) {
						case EntityType::Object:
							scenemanager.addEntity<Vertex, EntityType::Object>(entity.vertexPath, entity.fragmentPath, entity.texturePath, entity.modelPath, entity.flipTexture);
							entity.add = false;
							break;
						case EntityType::PBRObject:
							scenemanager.addEntity<Vertex, EntityType::PBRObject>(entity.vertexPath, entity.fragmentPath, entity.texturePaths, entity.modelPath, entity.flipTexture);
							entity.add = false;
							break;
						case EntityType::MatObject:
							scenemanager.addEntity<Vertex, EntityType::MatObject>(entity.vertexPath, entity.fragmentPath, entity.materialPath, entity.modelPath, entity.flipTexture);
							entity.add = false;
							break;
						case EntityType::Primitive:
							scenemanager.addEntity<Vertex, EntityType::Primitive>(entity.vertexPath, entity.fragmentPath, entity.primitiveType, "", entity.flipTexture);
							entity.add = false;
							break;
						case EntityType::Light:
							scenemanager.addEntity<Vertex, EntityType::Light>(entity.vertexPath, entity.fragmentPath, "", "", false);
							entity.add = false;
							break;
						case EntityType::Skybox:
							if (entity.skyboxPaths.size() != 6) {
								ImGui::Text("Missing skybox side");
								break;
							}
							else {
								scenemanager.addEntity<CubeVertex, EntityType::Skybox>(entity.vertexPath, entity.fragmentPath, entity.skyboxPaths, "", entity.flipTexture);
								entity.add = false;
								break;
							}
						}
					}
					ImGui::EndPopup();
				}
			}
		}

		ImGui::End();

		ImGui::Render();

		if (useRaytracer) {
			raytraceFrame();
		}
		else {
			drawFrame();
		}

		glfwPollEvents();
	}

	vkDeviceWaitIdle(device.getDevice());
}

void Engine::Core::Application::cleanup()
{
	vkDeviceWaitIdle(device.getDevice());

	raytrace.cleanup(device.getDevice());

	for (auto& fb : imguiFramebuffers) {
		vkDestroyFramebuffer(device.getDevice(), fb, nullptr);
	}
	vkDestroyRenderPass(device.getDevice(), imguiRenderPass, nullptr);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	swapchain.cleanupSwapChain(device, framebuffer);

	for (auto& scene : scenemanager.getScenes()) {
		auto& p = scene.model.pipeline;

		vkDestroyPipeline(device.getDevice(), p.getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(device.getDevice(), p.getPipelineLayout(), nullptr);
	}

	vkDestroyRenderPass(device.getDevice(), renderpass.getRenderPass(), nullptr);
	
	for(auto& scene : scenemanager.getScenes()) {
		auto& m = scene.model;

		int textureCount = m.texture.getTextureCount();

		for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
			if (i < m.texture.getUniformBuffers().size()) {
				vkDestroyBuffer(device.getDevice(), m.texture.getUniformBuffers()[i], nullptr);
				vkFreeMemory(device.getDevice(), m.texture.getUniformBuffersMemory()[i], nullptr);
			}
			if (i < m.texture.getSkyboxUniformBuffers().size()) {
				vkDestroyBuffer(device.getDevice(), m.texture.getSkyboxUniformBuffers()[i], nullptr);
				vkFreeMemory(device.getDevice(), m.texture.getSkyboxUniformBuffersMemory()[i], nullptr);
			}
		}

		vkDestroyDescriptorPool(device.getDevice(), m.descriptor.getDescriptorPool(), nullptr);
		if (textureCount == -1) {
			vkDestroySampler(device.getDevice(), m.texture.getTextureSampler(), nullptr);
			vkDestroyImageView(device.getDevice(), m.texture.getTextureImageView(), nullptr);

			vkDestroyImage(device.getDevice(), m.texture.getTextureImage(), nullptr);
			vkFreeMemory(device.getDevice(), m.texture.getTextureImageMemory(), nullptr);
		}
		else {
			for (int i = 0; i < textureCount; i++) {
				vkDestroySampler(device.getDevice(), m.texture.getTextureSampler(i), nullptr);
				vkDestroyImageView(device.getDevice(), m.texture.getTextureImageView(i), nullptr);

				vkDestroyImage(device.getDevice(), m.texture.getTextureImage(i), nullptr);
				vkFreeMemory(device.getDevice(), m.texture.getTextureImageMemory(i), nullptr);
			}
		}
	}
	vkDestroyDescriptorPool(device.getDevice(), imguiPool, nullptr);
	vkDestroyDescriptorSetLayout(device.getDevice(), renderpass.getDescriptorSetLayout(), nullptr);
	
	for (auto& scene : scenemanager.getScenes()) {
		auto& m = scene.model;
		vkDestroyBuffer(device.getDevice(), m.texture.getIndexBuffer(), nullptr);
		vkFreeMemory(device.getDevice(), m.texture.getIndexBufferMemory(), nullptr);
		vkDestroyBuffer(device.getDevice(), m.texture.getVertexBuffer(), nullptr);
		vkFreeMemory(device.getDevice(), m.texture.getVertexBufferMemory(), nullptr);
	}

	for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device.getDevice(), texture.getRenderFinishedSemaphores()[i], nullptr);
		vkDestroySemaphore(device.getDevice(), texture.getImageAvailableSemaphores()[i], nullptr);
		vkDestroyFence(device.getDevice(), texture.getInFlightFences()[i], nullptr);
	}

	vkDestroyCommandPool(device.getDevice(), commandbuffer.getCommandPool(), nullptr);

	vkDeviceWaitIdle(device.getDevice());

	vkDestroyDevice(device.getDevice(), nullptr);

	instance.DestroyDebugUtilsMessengerEXT(instance.getInstance(), instance.getDebugMessenger(), nullptr);

	vkDestroySurfaceKHR(instance.getInstance(), instance.getSurface(), nullptr);
	vkDestroyInstance(instance.getInstance(), nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

void Engine::Core::Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Engine::Core::Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Engine::Core::Application::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (isFocused) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::UP, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			camera.processKeyboard(Camera_Movement::DOWN, deltaTime);
		}
	}
}

void Engine::Core::Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
		isFocused = !isFocused;
	}
}

void Engine::Core::Application::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (isFocused) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera.processMouseMovement(xoffset, yoffset);
	}
}

void Engine::Core::Application::drawFrame()
{
	vkDeviceWaitIdle(device.getDevice());

	if (recreateSwapchainFlag) {
		swapchain.presentImmediate = !swapchain.presentImmediate;
		recreateSwapchain();
		recreateSwapchainFlag = !recreateSwapchainFlag;
		return;
	}

	VkResult fenceStatus = vkWaitForFences(device.getDevice(), 1, &texture.getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);

	if (fenceStatus != VK_SUCCESS) {
		std::cerr << "failed to wait for fence: " << fenceStatus << std::endl;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device.getDevice(), swapchain.getSwapchain(), UINT64_MAX, texture.getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image");

	glm::vec3 lightPos = glm::vec3(0.0f);
	glm::vec3 lightColor = glm::vec3(0.7f);

	std::vector<Engine::Graphics::LightBuffer> lights;

	for (auto& scene : scenemanager.getScenes()) {
		auto& m = scene.model;
		Engine::Graphics::LightBuffer light;

		if (m.type == EntityType::Light) {
			light.pos = glm::vec3(m.matrix[3]);
			light.color = m.color;

			lights.push_back(light);
		}
	}

	for (auto& scene : scenemanager.getScenes()) {
		auto& m = scene.model;
		if (m.type == EntityType::Skybox) {
			m.texture.updateSkyboxUniformBuffer(currentFrame, camera, swapchain.getSwapchainExtent());
		}
		else {
			m.texture.updateUniformBuffer(currentFrame, camera, swapchain.getSwapchainExtent(), m.matrix, m.color, lights);
		}
	}
	
	VkResult fencesReset = vkResetFences(device.getDevice(), 1, &texture.getInFlightFences()[currentFrame]);
	if (fencesReset != VK_SUCCESS) {
		throw std::runtime_error("failed to reset fences");
	}

	VkResult commandBufReset = vkResetCommandBuffer(commandbuffer.getCommandBuffers()[currentFrame], 0);
	if (commandBufReset != VK_SUCCESS) {
		throw std::runtime_error("failed to reset command buffers");
	}

	vkResetCommandBuffer(commandbuffer.getCommandBuffers()[currentFrame], 0);
	recordCommandBuffer(commandbuffer.getCommandBuffers()[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphore[] = { texture.getImageAvailableSemaphores()[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandbuffer.getCommandBuffers()[currentFrame];

	VkSemaphore signalSemaphore[] = { texture.getRenderFinishedSemaphores()[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphore;

	VkResult fenceCheck = vkGetFenceStatus(device.getDevice(), texture.getInFlightFences()[currentFrame]);

	VkResult res = vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, texture.getInFlightFences()[currentFrame]);

	if (res != VK_SUCCESS) {
		std::cerr << "vkQueueSubmit returned " << res << " (0x" << std::hex << res << std::dec << ")\n";
		throw std::runtime_error("failed to submit draw command buffer");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphore;

	VkSwapchainKHR swapChains[]{ swapchain.getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image");
	}

	currentFrame = (currentFrame + 1) % Engine::Settings::MAX_FRAMES_IN_FLIGHT;
}

void Engine::Core::Application::raytraceFrame()
{
	vkDeviceWaitIdle(device.getDevice());

	if (recreateSwapchainFlag) {
		swapchain.presentImmediate = !swapchain.presentImmediate;
		recreateSwapchain();
		recreateSwapchainFlag = !recreateSwapchainFlag;
		return;
	}

	if (raytrace.uboData.view != camera.GetViewMatrix() || !accumulateFrames) {
		raytrace.uboData.sampleCount = 1;
	}
	else {
		raytrace.uboData.sampleCount++;
	}

	raytrace.uboData.view = camera.GetViewMatrix();
	raytrace.uboData.proj = camera.GetProjectionMatrix();

	raytrace.updateUBO(device);

	VkResult fenceStatus = vkWaitForFences(device.getDevice(), 1, &texture.getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);
	if (fenceStatus != VK_SUCCESS) {
		std::cerr << "failed to wait for fence: " << fenceStatus << std::endl;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device.getDevice(), swapchain.getSwapchain(), UINT64_MAX, texture.getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image");
	}

	VkResult fencesReset = vkResetFences(device.getDevice(), 1, &texture.getInFlightFences()[currentFrame]);

	if (fencesReset != VK_SUCCESS) {
		throw std::runtime_error("failed to reset fences");
	}

	VkCommandBuffer commandBuffer = commandbuffer.getCommandBuffers()[currentFrame];
	VkResult commandBufferReset = vkResetCommandBuffer(commandBuffer, 0);
	if (commandBufferReset != VK_SUCCESS) {
		throw std::runtime_error("failed to reset command buffers");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	raytrace.traceRays(device.getDevice(), commandBuffer, swapchain.getSwapchainExtent(), swapchain.getSwapchainImages()[imageIndex], imageIndex);

	VkImageMemoryBarrier imguiBarrier{};
	imguiBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imguiBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imguiBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imguiBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	imguiBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imguiBarrier.image = swapchain.getSwapchainImages()[imageIndex];
	imguiBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imguiBarrier.subresourceRange.levelCount = 1;
	imguiBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imguiBarrier
	);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = imguiRenderPass;
	renderPassInfo.framebuffer = imguiFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchain.getSwapchainExtent();
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = nullptr;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier presentBarrier{};
	presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	presentBarrier.dstAccessMask = 0;
	presentBarrier.image = swapchain.getSwapchainImages()[imageIndex];
	presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	presentBarrier.subresourceRange.levelCount = 1;
	presentBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &presentBarrier
	);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphore[] = { texture.getImageAvailableSemaphores()[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphore[] = { texture.getRenderFinishedSemaphores()[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphore;

	VkResult res = vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, texture.getInFlightFences()[currentFrame]);
	if (res != VK_SUCCESS) {
		std::cerr << "vkQueueSubmit returned " << res << " (0x" << std::hex << res << std::dec << ")\n";
		throw std::runtime_error("failed to submit ray tracing command buffer");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphore;

	VkSwapchainKHR swapChains[] = { swapchain.getSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image");
	}

	currentFrame = (currentFrame + 1) % Engine::Settings::MAX_FRAMES_IN_FLIGHT;
}

void Engine::Core::Application::recreateSwapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device.getDevice());

	swapchain.cleanupSwapChain(device, framebuffer);

	swapchain.createSwapChain(window, instance, device);
	swapchain.createImageViews(device.getDevice());

	framebuffer.createColorResources(device, swapchain, sampler.getSamples());
	framebuffer.createDepthResources(device, swapchain, sampler.getSamples(), commandbuffer);
	framebuffer.createFramebuffers(device.getDevice(), swapchain, renderpass.getRenderPass());

	createImGuiRenderPass();
	createImGuiFramebuffers();
}

void Engine::Core::Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderpass.getRenderPass();
	renderPassInfo.framebuffer = swapchain.getSwapchainFramebuffers()[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchain.getSwapchainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.getSwapchainExtent().width;
	viewport.height = (float)swapchain.getSwapchainExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.getSwapchainExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	for (auto& scene : scenemanager.getScenes()) {
		auto& m = scene.model;
		auto& p = scene.model.pipeline;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p.getGraphicsPipeline());

		VkBuffer vertexBuffers[] = { m.texture.getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m.texture.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p.getPipelineLayout(), 0, 1, &m.descriptor.getDescriptorSets()[currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, m.indexCount, 1, 0, 0, 0);
	}

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Engine::Core::Application::createModel(int x)
{
	RTScene obj;
	MeshObject testModel;
	testModel.v = {
		{{-1.0f + x,  4.0f,  1.0f}, {}, {0.0f, 0.0f}},
		{{ 1.0f + x,  4.0f,  1.0f}, {}, {1.0f, 0.0f}},
		{{ 1.0f + x,  6.0f,  1.0f}, {}, {1.0f, 1.0f}},
		{{-1.0f + x,  6.0f,  1.0f}, {}, {0.0f, 1.0f}},

		{{ 1.0f + x,  4.0f, -1.0f}, {}, {0.0f, 0.0f}},
		{{-1.0f + x,  4.0f, -1.0f}, {}, {1.0f, 0.0f}},
		{{-1.0f + x,  6.0f, -1.0f}, {}, {1.0f, 1.0f}},
		{{ 1.0f + x,  6.0f, -1.0f}, {}, {0.0f, 1.0f}},

		{{-1.0f + x,  6.0f,  1.0f}, {}, {0.0f, 0.0f}},
		{{ 1.0f + x,  6.0f,  1.0f}, {}, {1.0f, 0.0f}},
		{{ 1.0f + x,  6.0f, -1.0f}, {}, {1.0f, 1.0f}},
		{{-1.0f + x,  6.0f, -1.0f}, {}, {0.0f, 1.0f}},

		{{-1.0f + x,  4.0f, -1.0f}, {}, {0.0f, 0.0f}},
		{{ 1.0f + x,  4.0f, -1.0f}, {}, {1.0f, 0.0f}},
		{{ 1.0f + x,  4.0f,  1.0f}, {}, {1.0f, 1.0f}},
		{{-1.0f + x,  4.0f,  1.0f}, {}, {0.0f, 1.0f}},

		{{-1.0f + x,  4.0f, -1.0f}, {}, {0.0f, 0.0f}},
		{{-1.0f + x,  4.0f,  1.0f}, {}, {1.0f, 0.0f}},
		{{-1.0f + x,  6.0f,  1.0f}, {}, {1.0f, 1.0f}},
		{{-1.0f + x,  6.0f, -1.0f}, {}, {0.0f, 1.0f}},

		{{ 1.0f + x,  4.0f,  1.0f}, {}, {0.0f, 0.0f}},
		{{ 1.0f + x,  4.0f, -1.0f}, {}, {1.0f, 0.0f}},
		{{ 1.0f + x,  6.0f, -1.0f}, {}, {1.0f, 1.0f}},
		{{ 1.0f + x,  6.0f,  1.0f}, {}, {0.0f, 1.0f}}
	};

	testModel.i = {
		0, 1, 2,  2, 3, 0,
		4, 5, 6,  6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	for (auto& v : testModel.v) {
		v.normal = { 0.0, 0.0, 0.0 };
	}

	for (uint32_t i = 0; i < testModel.i.size(); i += 3) {
		uint32_t i0 = testModel.i[i];
		uint32_t i1 = testModel.i[i + 1];
		uint32_t i2 = testModel.i[i + 2];

		glm::vec3 edge1 = testModel.v[i1].pos - testModel.v[i0].pos;
		glm::vec3 edge2 = testModel.v[i2].pos - testModel.v[i0].pos;
		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		testModel.v[i0].normal += normal;
		testModel.v[i1].normal += normal;
		testModel.v[i2].normal += normal;
	}

	for (auto& v : testModel.v) {
		v.normal = glm::normalize(v.normal);
	}

	VkDeviceSize vertexBufferSize = sizeof(testModel.v[0]) * testModel.v.size();
	framebuffer.createBuffer(device, vertexBufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		testModel.vb, testModel.vbm, testModel.v.data());

	VkDeviceSize indexBufferSize = sizeof(testModel.i[0]) * testModel.i.size();
	framebuffer.createBuffer(device, indexBufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		testModel.ib, testModel.ibm, testModel.i.data());

	obj.matrix = glm::mat4(1.0f);
	obj.obj = testModel;
	raytrace.models.push_back(obj);
}

void Engine::Core::Application::createImGuiRenderPass()
{
	VkAttachmentDescription attachment{};
	attachment.format = swapchain.getSwapchainImageFormat();
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &imguiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ImGui render pass");
	}
}

void Engine::Core::Application::createImGuiFramebuffers()
{
	imguiFramebuffers.resize(swapchain.getSwapchainFramebuffers().size());
	for (size_t i = 0; i < imguiFramebuffers.size(); i++) {
		VkImageView attachments[] = { swapchain.getSwapchainImageViews()[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = imguiRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain.getSwapchainExtent().width;
		framebufferInfo.height = swapchain.getSwapchainExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &imguiFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create ImGui framebuffer");
		}
	}
}
