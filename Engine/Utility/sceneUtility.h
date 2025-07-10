#ifndef SCENE_UTILITY
#define SCENE_UTILITY

#include "utility.h"
#include "texture.h"

struct RTScene {
	MeshObject obj;
	std::string name = "";
	bool markedForDeletion = false;
	glm::mat4 matrix;
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 color = glm::vec3(0.75f);
	std::optional<Engine::Graphics::Texture> albedo = std::nullopt;
	std::optional<Engine::Graphics::Texture> normal = std::nullopt;
	std::optional<Engine::Graphics::Texture> roughness = std::nullopt;
	std::optional<Engine::Graphics::Texture> metalness = std::nullopt;
	std::optional<Engine::Graphics::Texture> specular = std::nullopt;
	std::optional<Engine::Graphics::Texture> height = std::nullopt;
	std::optional<Engine::Graphics::Texture> ambientOcclusion = std::nullopt;
	int totalTextures = 0;
	bool hasTexture = false;
	bool showGizmo = false;
};

#endif