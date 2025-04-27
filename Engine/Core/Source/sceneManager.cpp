#include "sceneManager.h"

void Engine::Core::SceneManager::removeEntity(Scene scene, int index) {
	cleanup(scene);
	scenes.erase(scenes.begin() + index);
}

void Engine::Core::SceneManager::updateScene() {
	for (int i = 0; i < scenes.size(); i++) {
		ImGui::PushID(i);
		
		ImGui::Text("Entity Type: %s", entityString(scenes[i].model.type));

		glm::vec3& pos = scenes[i].model.position;
		glm::vec3& rot = scenes[i].model.rotation;
		glm::vec3& scale = scenes[i].model.scale;

		float posArr[3] = { pos.x, pos.y, pos.z };
		if (ImGui::SliderFloat3("Pos", posArr, -10, 10)) {
			pos = glm::vec3(posArr[0], posArr[1], posArr[2]);	
		}

		float rotArr[3] = { rot.x, rot.y, rot.z };
		if (ImGui::SliderFloat3("Rot", rotArr, -180.0f, 180.0f)) {
			rot = glm::vec3(rotArr[0], rotArr[1], rotArr[2]);
		}

		float scaleArr[3] = { scale.x, scale.y, scale.z };
		if (ImGui::SliderFloat3("Scale", scaleArr, 0.1, 10)) {
			scale = glm::vec3(scaleArr[0], scaleArr[1], scaleArr[2]);
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, pos);
		transform = glm::rotate(transform, glm::radians(rot.x), glm::vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(rot.y), glm::vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(rot.z), glm::vec3(0, 0, 1));
		transform = glm::scale(transform, scale);

		scenes[i].model.matrix = transform;

		if (ImGui::Button("Remove Entity")) {
			removeEntity(scenes[i], i);
		}

		ImGui::PopID();
		ImGui::Separator();
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
