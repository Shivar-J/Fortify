#ifndef SCENE_UTILITY
#define SCENE_UTILITY

#include "utility.h"
#include "animation.h"

struct RTScene {
	MeshObject obj;
	std::string name = "";
	bool markedForDeletion = false;
	glm::mat4 matrix;
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 color = glm::vec3(0.75f);
	std::optional<ImageResource*> albedo = std::nullopt;
	std::optional<ImageResource*> normal = std::nullopt;
	std::optional<ImageResource*> roughness = std::nullopt;
	std::optional<ImageResource*> metalness = std::nullopt;
	std::optional<ImageResource*> specular = std::nullopt;
	std::optional<ImageResource*> height = std::nullopt;
	std::optional<ImageResource*> ambientOcclusion = std::nullopt;
	int totalTextures = 0;
	bool hasTexture = false;
	bool showGizmo = false;
	Engine::Graphics::Animation animation;

	void textureCleanup() {
		if (albedo.has_value()) resources->destroy(albedo.value());
		if (normal.has_value()) resources->destroy(normal.value());
		if (roughness.has_value()) resources->destroy(roughness.value());
		if (metalness.has_value()) resources->destroy(metalness.value());
		if (specular.has_value()) resources->destroy(specular.value());
		if (height.has_value()) resources->destroy(height.value());
		if (ambientOcclusion.has_value()) resources->destroy(ambientOcclusion.value());
	}
};

#endif