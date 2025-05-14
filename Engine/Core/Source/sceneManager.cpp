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

	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].markedForDeletion) {
			removeEntity(scenes[i], i);
		}
	}

	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].model.type != EntityType::Skybox) {
			ImGui::PushID(i);

			ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

			int textureCount = static_cast<int>(scenes[i].model.texture.getTextureCount());
			if (scenes[i].model.type != EntityType::Primitive) {
				if (textureCount == -1) {
					if (scenes[i].model.textureIDs.count(0) == 0) {
						VkDescriptorSet textureID = ImGui_ImplVulkan_AddTexture(scenes[i].model.texture.getTextureSampler(), scenes[i].model.texture.getTextureImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						scenes[i].model.textureIDs[0] = textureID;
					}
					ImGui::Image((ImTextureID)scenes[i].model.textureIDs[0], ImVec2(64, 64));
				}
				else {
					scenes[i].model.textureIDs.reserve(textureCount);
					int index = 0;
					for (auto& texturePair : scenes[i].model.texturePaths) {
						if (scenes[i].model.textureIDs.count(index) == 0) {
							VkDescriptorSet textureID = ImGui_ImplVulkan_AddTexture(scenes[i].model.texture.getTextureSampler(index), scenes[i].model.texture.getTextureImageView(index), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
							scenes[i].model.textureIDs[index] = textureID;
						}
						ImGui::Image((ImTextureID)scenes[i].model.textureIDs[index], ImVec2(64, 64));
						ImGui::SameLine();
						ImGui::Text("%s", textureString(texturePair.first));
						index++;
					}
				}
			}

			if (scenes[i].model.type == EntityType::Primitive) {
				ImGui::SameLine();
				ImGui::Text("%s", primitiveString(scenes[i].model.primitiveType));
				//ImGui::ColorPicker3("Color", scenes[i].model.color);
			}

			glm::mat4& matrix = scenes[i].model.matrix;

			static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
			static ImGuizmo::MODE currentMode = ImGuizmo::LOCAL;
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

	for (auto& texturePair : m.textureIDs) {
		ImGui_ImplVulkan_RemoveTexture(texturePair.second);
	}

	m.textureIDs.clear();

	vkDestroyPipeline(device.getDevice(), p.getGraphicsPipeline(), nullptr);
	vkDestroyPipelineLayout(device.getDevice(), p.getPipelineLayout(), nullptr);

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

	vkDestroyBuffer(device.getDevice(), m.texture.getIndexBuffer(), nullptr);
	vkFreeMemory(device.getDevice(), m.texture.getIndexBufferMemory(), nullptr);
	vkDestroyBuffer(device.getDevice(), m.texture.getVertexBuffer(), nullptr);
	vkFreeMemory(device.getDevice(), m.texture.getVertexBufferMemory(), nullptr);
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

bool Engine::Core::SceneManager::checkExtension(const std::string path, const std::string ext)
{
	return path.size() >= ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0;
}
