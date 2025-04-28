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
		if (scenes[i].model.type != EntityType::Skybox) {
			ImGui::PushID(i);

			ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

			glm::mat4& matrix = scenes[i].model.matrix;

			static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
			static ImGuizmo::MODE currentMode = ImGuizmo::LOCAL;

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
				removeEntity(scenes[i], i);
			}

			ImGui::PopID();
			ImGui::Separator();
		}
		else {
			ImGui::PushID(i);

			ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

			if (ImGui::Button("Remove Entity")) {
				removeEntity(scenes[i], i);
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
		default: return "Unknown";
	}
}

bool Engine::Core::SceneManager::checkExtension(const std::string path, const std::string ext)
{
	return path.size() >= ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0;
}
