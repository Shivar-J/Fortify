#ifndef SCENE_H
#define SCENE_H

#include "utility.h"
#include "pipeline.h"
#include "texture.h"
#include "descriptorSets.h"
#include "commandBuffer.h"
#include "sampler.h"
#include "string"
#include "camera.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "imfilebrowser.h"

enum class EntityType {
	Object,
	Skybox,
	UI,
	Light,
	Terrain,
	Particle,
	PBRObject
};

enum class ShaderType {
	Vertex,
	Fragment
};

struct Entity {
	std::string vertexPath = "";
	std::string fragmentPath = "";
	std::string texturePath = "";
	std::unordered_map<PBRTextureType, std::string> texturePaths;
	std::string modelPath = "";
	bool flipTexture = false;
	bool add = false;
	EntityType type = EntityType::Object;
	PBRTextureType textureType = PBRTextureType::Albedo;
};


struct ObjectTag{};
struct PBRObjectTag{};
struct SkyboxTag{};

struct Model {
	Engine::Graphics::Pipeline pipeline;
	Engine::Graphics::Texture texture;
	Engine::Graphics::DescriptorSets descriptor;
	uint32_t indexCount;
	glm::mat4 matrix;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0f);
	int32_t pipelineIndex;
	EntityType type;
	std::unordered_map<PBRTextureType, std::string> texturePaths;
	std::unordered_map<int, VkDescriptorSet> textureIDs;
	bool showGizmo = false;
};

struct Scene {
	Model model;
	std::string vertexShader;
	std::string fragmentShader;
	bool isVisible = true;
	bool markedForDeletion = false;
};

namespace Engine::Core {
	class SceneManager {
	public:
		SceneManager(
			Engine::Graphics::Device& device, 
			Engine::Graphics::Sampler& sampler,
			Engine::Graphics::RenderPass& renderpass, 
			Engine::Graphics::CommandBuffer& commandbuffer, 
			Engine::Graphics::FrameBuffer& framebuffer, 
			Engine::Graphics::Swapchain& swapchain,
			Engine::Core::Camera& camera
		) : device(device), sampler(sampler), renderpass(renderpass), commandbuffer(commandbuffer), framebuffer(framebuffer), swapchain(swapchain), camera(camera) {}

		template<EntityType Entity> struct TagFromEntityType {};
		template<> struct TagFromEntityType<EntityType::Object> { using type = ObjectTag; };
		template<> struct TagFromEntityType<EntityType::PBRObject> { using type = PBRObjectTag; };
		template<> struct TagFromEntityType<EntityType::Skybox> { using type = SkyboxTag; };

		template<EntityType Entity>
		using tag = typename TagFromEntityType<Entity>::type;

		template<typename VertexType, EntityType Entity>
		void addEntity(const std::string vertexShaderPath, const std::string fragmentShaderPath, const auto& textureParam, const std::string modelPath, bool flipTexture) {
			addEntityImplementation<VertexType>(tag<Entity>{}, vertexShaderPath, fragmentShaderPath, textureParam, modelPath, flipTexture);
		}

		template<typename VertexType>
		void addEntityImplementation(ObjectTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string texturePath, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, false);
			m.texture.createTextureImage(texturePath, device, commandbuffer, framebuffer, sampler, flipTexture);
			m.matrix = glm::mat4(1.0f);

			if (!modelPath.empty())
				m.texture.loadModel(modelPath);
			m.texture.createTextureImageView(swapchain, device.getDevice(), false);
			m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);
			m.type = EntityType::Object;
			m.texture.createVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createIndexBuffer(device, commandbuffer, framebuffer);
			m.texture.createUniformBuffers(device, framebuffer);
			m.indexCount = static_cast<uint32_t>(m.texture.getIndices().size());

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), false);

			scene.model = m;

			scenes.push_back(scene);
		}

		template<typename VertexType>
		void addEntityImplementation(PBRObjectTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::unordered_map<PBRTextureType, std::string>& texturePaths, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.texturePaths = texturePaths;
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, false);
			
			if (texturePaths.count(PBRTextureType::Albedo)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Albedo), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}

			if (texturePaths.count(PBRTextureType::Normal)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Normal), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}

			if (texturePaths.count(PBRTextureType::Roughness)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Roughness), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}

			if (texturePaths.count(PBRTextureType::Metalness)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Metalness), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}
			
			if (texturePaths.count(PBRTextureType::AmbientOcclusion)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::AmbientOcclusion), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}

			if (texturePaths.count(PBRTextureType::Specular)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Specular), device, commandbuffer, framebuffer, sampler, flipTexture, true);
				m.texture.createTextureImageView(swapchain, device.getDevice(), false, true);
				m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false, true);
			}

			if (!modelPath.empty()) {
				m.texture.loadModel(modelPath);
			}

			m.type = EntityType::PBRObject;

			m.texture.createVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createIndexBuffer(device, commandbuffer, framebuffer);
			m.texture.createUniformBuffers(device, framebuffer);
			m.indexCount = static_cast<uint32_t>(m.texture.getIndices().size());
			m.matrix = glm::mat4(1.0f);

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), false, texturePaths);

			scene.model = m;

			scenes.push_back(scene);
		}

		template<typename VertexType>
		void addEntityImplementation(SkyboxTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::vector<std::string>& skyboxPaths, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, true);

			m.texture.createCubemap(skyboxPaths, device, commandbuffer, framebuffer, sampler, false);
			m.texture.createCube();
			m.texture.createCubeVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createCubeIndexBuffer(device, commandbuffer, framebuffer);
			m.matrix = glm::mat4(1.0f);
			m.indexCount = static_cast<uint32_t>(m.texture.getCubeIndices().size());
			m.texture.createSkyboxUniformBuffers(device, framebuffer);
			m.type = EntityType::Skybox;
			m.texture.createTextureImageView(swapchain, device.getDevice(), true);
			m.texture.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), true);

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), true);

			scene.model = m;

			scenes.push_back(scene);
		}

		void removeEntity(Scene scene, int index);
		void updateScene();
		void cleanup(Scene scene);
		const char* entityString(EntityType type);
		const char* textureString(PBRTextureType type);

		bool checkExtension(const std::string path, const std::string ext);

		std::vector<Scene> getScenes() const { return scenes; }

	private:
		std::vector<Scene> scenes;
		Engine::Graphics::Device& device;
		Engine::Graphics::RenderPass& renderpass;
		Engine::Graphics::Sampler& sampler;
		Engine::Graphics::CommandBuffer& commandbuffer;
		Engine::Graphics::FrameBuffer& framebuffer;
		Engine::Graphics::Swapchain& swapchain;
		Engine::Core::Camera& camera;
	};
}

#endif
