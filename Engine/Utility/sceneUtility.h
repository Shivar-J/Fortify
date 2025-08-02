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
	bool hasTexture = false;
	bool hasAnimation = false;
	bool showGizmo = false;
	bool isEmissive = false;
	Engine::Graphics::Animation animation;
};

#endif