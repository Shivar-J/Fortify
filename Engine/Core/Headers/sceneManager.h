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
#include "raytracing.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "imfilebrowser.h"

struct Entity {
	std::string vertexPath = "";
	std::string fragmentPath = "";
	std::string texturePath = "";
	std::unordered_map<PBRTextureType, std::string> texturePaths;
	std::vector<std::string> skyboxPaths;
	std::string materialPath = "";
	std::string modelPath = "";
	PrimitiveType primitiveType = PrimitiveType::Cube;
	bool flipTexture = false;
	bool add = false;
	EntityType type = EntityType::Object;
	PBRTextureType textureType = PBRTextureType::Albedo;
};

struct ObjectTag{};
struct PBRObjectTag{};
struct SkyboxTag{};
struct MatObjectTag{};
struct PrimitiveTag{};
struct LightingTag{};

template<EntityType E>
struct TagFromEntityType;

template<>
struct TagFromEntityType<EntityType::Object> { using type = ObjectTag; };

template<>
struct TagFromEntityType<EntityType::PBRObject> { using type = PBRObjectTag; };

template<>
struct TagFromEntityType<EntityType::Skybox> { using type = SkyboxTag; };

template<>
struct TagFromEntityType<EntityType::MatObject> { using type = MatObjectTag; };

template<>
struct TagFromEntityType<EntityType::Primitive> { using type = PrimitiveTag; };

template<>
struct TagFromEntityType<EntityType::Light> { using type = LightingTag; };

struct Model {
	Engine::Graphics::Pipeline pipeline;
	Engine::Graphics::Texture texture;
	Engine::Graphics::DescriptorSets descriptor;
	uint32_t indexCount;
	glm::mat4 matrix;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 color = glm::vec3(0.7f);
	int32_t pipelineIndex;
	EntityType type;
	PrimitiveType primitiveType = PrimitiveType::Cube;
	std::unordered_map<PBRTextureType, std::string> texturePaths;
	std::unordered_map<int, VkDescriptorSet> textureIDs;
	bool showGizmo = false;
	bool hasTexture = true;
};

struct Scene {
	Model model;
	std::string vertexShader;
	std::string fragmentShader;
	bool isVisible = true;
	bool markedForDeletion = false;
	std::string name = "";
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
			Engine::Core::Camera& camera,
			Engine::Graphics::Raytracing& raytrace
		) : device(device), sampler(sampler), renderpass(renderpass), commandbuffer(commandbuffer), framebuffer(framebuffer), swapchain(swapchain), camera(camera), raytrace(raytrace) {}

		template<EntityType E>
		using Tag = TagFromEntityType<E>;

		/*
		template<EntityType Entity> struct TagFromEntityType {};
		template<> struct TagFromEntityType<EntityType::Object> { using type = ObjectTag; };
		template<> struct TagFromEntityType<EntityType::PBRObject> { using type = PBRObjectTag; };
		template<> struct TagFromEntityType<EntityType::Skybox> { using type = SkyboxTag; };
		template<> struct TagFromEntityType<EntityType::MatObject> { using type = MatObjectTag; };
		template<> struct TagFromEntityType<EntityType::Primitive> { using type = PrimitiveTag; };
		template<> struct TagFromEntityType<EntityType::Light> { using type = LightingTag; };
		*/

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
			
			if (!modelPath.empty())
				m.texture.loadModel(modelPath);

			m.texture.createTextureImage(texturePath, device, commandbuffer, framebuffer, sampler, flipTexture, false, false, true);
			m.type = EntityType::Object;
			m.matrix = glm::mat4(1.0f);
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
			
			if (texturePaths.contains(PBRTextureType::Albedo)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Albedo), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
			}

			if (texturePaths.contains(PBRTextureType::Normal)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Normal), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
			}

			if (texturePaths.contains(PBRTextureType::Roughness)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Roughness), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
			}

			if (texturePaths.contains(PBRTextureType::Metalness)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Metalness), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
			}
			
			if (texturePaths.contains(PBRTextureType::AmbientOcclusion)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::AmbientOcclusion), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
			}

			if (texturePaths.contains(PBRTextureType::Specular)) {
				m.texture.createTextureImage(texturePaths.at(PBRTextureType::Specular), device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
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

			if (m.texture.skyboxUniformResources.empty()) {
				m.texture.createCubemap(skyboxPaths, device, commandbuffer, framebuffer, sampler, false);
			}

			m.texture.createSkybox();
			m.texture.createCubeVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createCubeIndexBuffer(device, commandbuffer, framebuffer);
			m.matrix = glm::mat4(1.0f);
			m.indexCount = static_cast<uint32_t>(m.texture.getCubeIndices().size());
			m.texture.createSkyboxUniformBuffers(device, framebuffer);
			m.type = EntityType::Skybox;

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), true);

			scene.model = m;

			scenes.insert(scenes.begin(), scene);
		}

		template<typename VertexType>
		void addEntityImplementation(MatObjectTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string materialPath, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, false);			
			
			if (!modelPath.empty()) {
				m.texture.loadModel(modelPath, materialPath);
			}

			std::string baseDir = std::filesystem::path(materialPath).parent_path().string();
			std::unordered_map<PBRTextureType, std::string> paths;

			for (auto& mat : m.texture.getMaterials()) {
				if (!mat.diffusePath.empty()) {
					std::string path = baseDir + "/" + mat.diffusePath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::Albedo, path });
				}

				if (!mat.normalPath.empty()) {
					std::string path = baseDir + "/" + mat.normalPath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::Normal, path });
				}

				if (!mat.roughnessPath.empty()) {
					std::string path = baseDir + "/" + mat.roughnessPath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::Roughness, path });
				}

				if (!mat.metalnessPath.empty()) {
					std::string path = baseDir + "/" + mat.metalnessPath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::Metalness, path });
				}

				if (!mat.aoPath.empty()) {
					std::string path = baseDir + "/" + mat.aoPath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::AmbientOcclusion, path });
				}

				if (!mat.specularPath.empty()) {
					std::string path = baseDir + "/" + mat.specularPath;
					m.texture.createTextureImage(path, device, commandbuffer, framebuffer, sampler, flipTexture, true, false, true);
					paths.insert({ PBRTextureType::Specular, path });
				}
			}

			m.type = EntityType::MatObject;
			m.texturePaths = paths;

			m.texture.createVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createIndexBuffer(device, commandbuffer, framebuffer);
			m.texture.createUniformBuffers(device, framebuffer);
			m.indexCount = static_cast<uint32_t>(m.texture.getIndices().size());
			m.matrix = glm::mat4(1.0f);

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), false, paths);
		
			scene.model = m;

			scenes.push_back(scene);
		}

		template<typename VertexType>
		void addEntityImplementation(PrimitiveTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const PrimitiveType primitiveType, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.primitiveType = primitiveType;
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, true);
			m.hasTexture = false;

			if (primitiveType == PrimitiveType::Cube) {
				m.texture.createCube();
				m.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5));
			}
			else if (primitiveType == PrimitiveType::Plane) {
				m.texture.createPlane();
				m.matrix = glm::translate(glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0)), glm::vec3(5.0, 5.0, 5.0)), glm::vec3(0.0, 0.0, -1.0));
			}
			else if (primitiveType == PrimitiveType::Sphere) {
				m.texture.createSphere();
				m.matrix = glm::mat4(1.0f);
			}

			m.texture.createVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createIndexBuffer(device, commandbuffer, framebuffer);
			m.indexCount = static_cast<uint32_t>(m.texture.getIndices().size());
			m.texture.createUniformBuffers(device, framebuffer);
			m.type = EntityType::Primitive;
			
			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), false, {}, false);
			
			scene.model = m;

			scenes.push_back(scene);
		}

		template<typename VertexType>
		void addEntityImplementation(LightingTag, const std::string vertexShaderPath, const std::string fragmentShaderPath, const std::string texturePath, const std::string modelPath, bool flipTexture) {
			Scene scene;
			scene.vertexShader = vertexShaderPath;
			scene.fragmentShader = fragmentShaderPath;

			Model m{};
			m.pipeline.createGraphicsPipeline<VertexType>(scene.vertexShader, scene.fragmentShader, device.getDevice(), sampler.getSamples(), renderpass, true);

			//change this to user object
			m.texture.createCube();
			m.matrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.5)), glm::vec3(0.0, 5.0, 0.0));
			m.type = EntityType::Light;
			m.hasTexture = false;

			m.texture.createVertexBuffer(device, commandbuffer, framebuffer);
			m.texture.createIndexBuffer(device, commandbuffer, framebuffer);
			m.indexCount = static_cast<uint32_t>(m.texture.getIndices().size());
			m.texture.createUniformBuffers(device, framebuffer);

			m.descriptor.createDescriptorPool(device.getDevice());
			m.descriptor.createDescriptorSets(device.getDevice(), m.texture, renderpass.getDescriptorSetLayout(), false, {}, false);

			scene.model = m;

			scenes.push_back(scene);
		}

		void removeEntity(const Scene &scene, int index);
		void updateScene();
		void cleanup(Scene scene) const;

		static const char* entityString(EntityType type);

		static const char* textureString(PBRTextureType type);

		static const char* primitiveString(PrimitiveType type);
		bool hasSkybox() const;
		void setShaderPaths(const std::vector<const char*> &paths);
		std::vector<const char*> getShaderPaths() const { return shaderPaths; }

		static bool checkExtension(const std::string &path, const std::string &ext);

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
		Engine::Graphics::Raytracing& raytrace;
		std::vector<const char*> shaderPaths;
	};
}

#endif
