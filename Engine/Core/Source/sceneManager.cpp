#include "sceneManager.h"

void Engine::Core::SceneManager::removeEntity(Scene scene, int index) {
	cleanup(scene);
	scenes.erase(scenes.begin() + index);
}

void Engine::Core::SceneManager::updateScene() {
	ImGuizmo::SetOrthographic(true);
	ImGuizmo::BeginFrame();
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

	std::unordered_map<EntityType, int> entityCount = {
		{ EntityType::Object, 0 },
		{ EntityType::UI, 0 },
		{ EntityType::Light, 0 },
		{ EntityType::Terrain, 0 },
		{ EntityType::Particle, 0 },
		{ EntityType::PBRObject, 0 },
		{ EntityType::MatObject, 0 },
		{ EntityType::Primitive, 0 },
	};

	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].model.type != EntityType::Skybox) {
			entityCount[scenes[i].model.type] += 1;
		}
		if (scenes[i].name.empty() && scenes[i].model.type != EntityType::Skybox) {
			scenes[i].name = static_cast<std::string>(entityString(scenes[i].model.type)) + " " + std::to_string(entityCount[scenes[i].model.type]);
		}
		if (scenes[i].markedForDeletion) {
			removeEntity(scenes[i], i);
		}
	}

	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].model.type != EntityType::Skybox) {
			ImGui::PushID(i);

			ImGui::Text("%s", scenes[i].name.c_str());
			ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

			if (scenes[i].model.type == EntityType::Primitive) {
				ImGui::SameLine();
				ImGui::Text("%s", primitiveString(scenes[i].model.primitiveType));
			}

			ImGui::NewLine();
			ImGui::ColorPicker3("Color", glm::value_ptr(scenes[i].model.color));

			int textureCount = static_cast<int>(scenes[i].model.texture.getTextureCount());
			if (scenes[i].model.hasTexture) {
				if (textureCount == -1) {
					if (scenes[i].model.textureIDs.count(0) == 0) {
						VkDescriptorSet textureID = ImGui_ImplVulkan_AddTexture(scenes[i].model.texture.textureResource->sampler, scenes[i].model.texture.textureResource->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						scenes[i].model.textureIDs[0] = textureID;
					}
					ImGui::Image((ImTextureID)scenes[i].model.textureIDs[0], ImVec2(64, 64));
				}
				else {
					scenes[i].model.textureIDs.reserve(textureCount);
					int index = 0;
					for (auto& texturePair : scenes[i].model.texturePaths) {
						if (scenes[i].model.textureIDs.count(index) == 0) {
							VkDescriptorSet textureID = ImGui_ImplVulkan_AddTexture(scenes[i].model.texture.textureResources[index]->sampler, scenes[i].model.texture.textureResources[index]->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
							scenes[i].model.textureIDs[index] = textureID;
						}
						ImGui::Image((ImTextureID)scenes[i].model.textureIDs[index], ImVec2(64, 64));
						ImGui::SameLine();
						ImGui::Text("%s", textureString(texturePair.first));
						index++;
					}
				}
			}

			glm::mat4& matrix = scenes[i].model.matrix;

			static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
			static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;
			ImGui::NewLine();
			ImGui::Checkbox("Show Gizmo", &scenes[i].model.showGizmo);

			if (scenes[i].model.showGizmo) {
				if (ImGui::RadioButton("Translate", currentOp == ImGuizmo::TRANSLATE)) {
					currentOp = ImGuizmo::TRANSLATE;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Rotate", currentOp == ImGuizmo::ROTATE)) {
					currentOp = ImGuizmo::ROTATE;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale", currentOp == ImGuizmo::SCALE)) {
					currentOp = ImGuizmo::SCALE;
				}

				ImGuizmo::Manipulate(glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjectionMatrix()), currentOp, currentMode, glm::value_ptr(matrix));
			}

			if (ImGui::Button("Remove Entity")) {
				scenes[i].markedForDeletion = true;
			}

			ImGui::PopID();
			ImGui::Separator();
		}
		else {
			ImGui::PushID(i);

			ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

			if (ImGui::Button("Remove Entity")) {
				scenes[i].markedForDeletion = true;
			}

			ImGui::PopID();
			ImGui::Separator();
		}
	}
}

void Engine::Core::SceneManager::cleanup(Scene scene)
{
	vkQueueWaitIdle(device.getGraphicsQueue());
	vkDeviceWaitIdle(device.getDevice());

	auto& p = scene.model.pipeline;
	auto& m = scene.model;
	int textureCount = m.texture.getTextureCount();

	if (m.hasTexture) {
		for (auto& texturePair : m.textureIDs) {
			ImGui_ImplVulkan_RemoveTexture(texturePair.second);
		}
	}

	m.textureIDs.clear();

	vkDestroyPipeline(device.getDevice(), p.getGraphicsPipeline(), nullptr);
	vkDestroyPipelineLayout(device.getDevice(), p.getPipelineLayout(), nullptr);

	for (size_t i = 0; i < Engine::Settings::MAX_FRAMES_IN_FLIGHT; i++) {
		if (i < m.texture.uniformResources.size()) {
			resources->destroy(m.texture.uniformResources[i], device.getDevice());
		}
		if (i < m.texture.skyboxUniformResources.size()) {
			resources->destroy(m.texture.skyboxUniformResources[i], device.getDevice());
		}
	}

	vkDestroyDescriptorPool(device.getDevice(), m.descriptor.getDescriptorPool(), nullptr);

	if(m.hasTexture) {
		if (textureCount == -1) {
			resources->destroy(m.texture.textureResource, device.getDevice());
		}
		else {
			for (int i = 0; i < textureCount; i++) {
				resources->destroy(m.texture.textureResources[i], device.getDevice());
			}
		}
	}

	resources->destroy(m.texture.vertexResource, device.getDevice());
	resources->destroy(m.texture.indexResource, device.getDevice());
}

const char* Engine::Core::SceneManager::entityString(EntityType type)
{
	switch (type) {
		case EntityType::Object: return "Object";
		case EntityType::Skybox: return "Skybox";
		case EntityType::UI: return "UI";
		case EntityType::Light: return "Light";
		case EntityType::Terrain: return "Terrain";
		case EntityType::Particle: return "Particle";
		case EntityType::PBRObject: return "PBR Object";
		case EntityType::MatObject: return "Material Object";
		case EntityType::Primitive: return "Primitive";
		default: return "Unknown";
	}
}

const char* Engine::Core::SceneManager::textureString(PBRTextureType type)
{
	switch (type) {
		case PBRTextureType::Albedo: return "Albedo";
		case PBRTextureType::Normal: return "Normal";
		case PBRTextureType::Roughness: return "Roughness";
		case PBRTextureType::Metalness: return "Metalness";
		case PBRTextureType::AmbientOcclusion: return "Ambient Occlusion";
		case PBRTextureType::Specular: return "Specular";
		default: return "Unknown";
	}
}

const char* Engine::Core::SceneManager::primitiveString(PrimitiveType type) {
	switch (type) {
	case PrimitiveType::Cube: return "Cube";
	case PrimitiveType::Sphere: return "Sphere";
	case PrimitiveType::Plane: return "Plane";
	default: return "Unknown";
	}
}

bool Engine::Core::SceneManager::hasSkybox() {
	for (auto& scene : scenes) {
		if (scene.model.type == EntityType::Skybox) {
			return true;
		}
	}

	return false;
}

void Engine::Core::SceneManager::setShaderPaths(std::vector<const char*> paths)
{
	shaderPaths = paths;
}

bool Engine::Core::SceneManager::checkExtension(const std::string path, const std::string ext)
{
	return path.size() >= ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0;
}
