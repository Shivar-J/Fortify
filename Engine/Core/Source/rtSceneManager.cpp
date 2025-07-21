#include "rtSceneManager.h"

#include "device.h"
#include "sampler.h"
#include "commandBuffer.h"
#include "frameBuffer.h"
#include "swapchain.h"
#include "camera.h"
#include "texture.h"
#include "raytracing.h"

void Engine::Core::RT::SceneManager::add(const std::string& texturePath) {
    std::filesystem::path path = texturePath;

    auto scene = std::make_shared<RTScene>();
    scene->obj = texture.loadModelRT(texturePath, device, framebuffer);
    scene->matrix = glm::mat4(1.0f);
    scene->name = path.filename().string();
    scene->obj.path = texturePath;

    std::vector<Keyframe> keyframes;
    constexpr float theta = glm::two_pi<float>() / 10.0f;
    constexpr float phi = glm::pi<float>() / 20.0f;
    int radius = 1 + (rand() % 5);

    for (int i = 0; i <= 64; ++i) {
        float t = (i / static_cast<float>(64)) * 10.0f;
        float angleTheta = theta * t;
        float anglePhi = glm::half_pi<float>() + std::sin(phi * t);

        glm::vec3 position = glm::vec3(
            radius * std::sin(anglePhi) * std::cos(angleTheta),
            radius * std::cos(anglePhi),
            radius * std::sin(anglePhi) * std::sin(angleTheta)
        );

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -angleTheta, glm::vec3(0, 1, 0));

        glm::mat4 model = translation * rotation;

        keyframes.push_back({ t, model });
    }

    scene->animation = Engine::Graphics::Animation(keyframes, true);
    scene->animation.setTime(static_cast<float>(rand()) / RAND_MAX * scene->animation.getDuration());

    std::vector<std::string> texturePaths;
    texturePaths = Engine::Utility::getAllPathsFromPath(path.parent_path().string() + "/", Engine::Utility::imageFileTypes);

    if(texturePaths.size() == 1) {
        scene->albedo = texture.createImageResource(texturePaths[0], device, commandbuffer, framebuffer, sampler, false, false, false, true);
    } else if(texturePaths.size() > 1) {
        for(auto& file : texturePaths) {
            if(file.find("albedo") != std::string::npos || file.find("diffuse") != std::string::npos) {
                scene->albedo = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("normal") != std::string::npos) {
                scene->normal = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("roughness") != std::string::npos) {
                scene->roughness = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("metalness") != std::string::npos) {
                scene->metalness = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("specular") != std::string::npos) {
                scene->specular = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("height") != std::string::npos) {
                scene->height= texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            }
            else if(file.find("ambient_occlusion") != std::string::npos) {
                scene->ambientOcclusion = texture.createImageResource(file, device, commandbuffer, framebuffer, sampler, false, false, false, true);
            } else {
                std::cout << "Textures found but naming convention not followed" << std::endl;
                // manually add textures
            }
        }
    }

    scene->totalTextures = texturePaths.size();

    scenes.push_back(scene);
}

void Engine::Core::RT::SceneManager::remove(int index)
{
    if (index >= 0 && index < scenes.size()) {
        scenes[index]->obj.destroy(device.getDevice());
        scenes[index]->textureCleanup();
        scenes.erase(scenes.begin() + index);
        raytrace.sceneUpdated = true;
    }
}

void Engine::Core::RT::SceneManager::pushToAccelerationStructure(std::vector<std::shared_ptr<RTScene>>& dst) {
    dst = scenes;
}

void Engine::Core::RT::SceneManager::updateScene(float deltaTime) {
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

    for (int i = 0; i < scenes.size(); i++) {
        if (scenes[i]->markedForDeletion) {
            remove(i);
        }
    }

    int i = 0;
    bool transformChanged = false;
    for(auto& scene : scenes) {
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Text("%s", scene->name.c_str());

        glm::mat4& matrix = scene->matrix;

        static ImGuizmo::OPERATION currOp = ImGuizmo::TRANSLATE;
        static ImGuizmo::MODE currMode = ImGuizmo::WORLD;

        ImGui::Checkbox("Show Gizmo", &scene->showGizmo);

        if (scene->showGizmo) {
            if (ImGui::RadioButton("Translate", currOp == ImGuizmo::TRANSLATE)) {
                currOp = ImGuizmo::TRANSLATE;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate", currOp == ImGuizmo::ROTATE)) {
                currOp = ImGuizmo::ROTATE;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale", currOp == ImGuizmo::SCALE)) {
                currOp = ImGuizmo::SCALE;
            }

            if (ImGuizmo::Manipulate(glm::value_ptr(camera.GetViewMatrix()), glm::value_ptr(camera.GetProjectionMatrix()), currOp, currMode, glm::value_ptr(matrix))) {
                transformChanged = true;
            }
        }

        if (ImGui::Button("Remove Entity")) {
            scene->markedForDeletion = true;
        }

        glm::mat4 oldMatrix = scene->matrix;

        scene->animation.update(deltaTime);
        scene->matrix = scene->animation.currentTransform();

        if (oldMatrix != scene->matrix) {
            transformChanged = true;
        }

        ImGui::PopID();

        i++;
    }

    if (transformChanged) {
        for (int i = 0; i < scenes.size(); i++) {
            raytrace.models[i]->matrix = scenes[i]->matrix;
        }
        raytrace.updateTopLevelAccelerationStructure(device, framebuffer, commandbuffer, true);
        raytrace.uboData.sampleCount = 1;
    }
}