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

    RTScene scene;
    scene.obj = texture.loadModelRT(texturePath, device, framebuffer);
    scene.matrix = glm::mat4(1.0f);
    scene.name = path.filename().string();

    std::vector<std::string> texturePaths;
    texturePaths = Engine::Utility::getAllPathsFromPath(path.parent_path().string() + "/", Engine::Utility::imageFileTypes);

    if(texturePaths.size() == 1) {
        Engine::Graphics::Texture albedo;
        albedo.createTextureImage(texturePaths[0], device, commandbuffer, framebuffer, sampler, false, false);
        albedo.createTextureImageView(swapchain, device.getDevice(), false);
        albedo.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

        scene.albedo = albedo;
    } else if(texturePaths.size() > 1) {
        for(auto& file : texturePaths) {
            Engine::Graphics::Texture temp;

            if(file.find("albedo") != std::string::npos || file.find("diffuse") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.albedo = std::move(temp);
            }
            else if(file.find("normal") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.normal = std::move(temp);
            }
            else if(file.find("roughness") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.roughness = std::move(temp);
            }
            else if(file.find("metalness") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.metalness = std::move(temp);
            }
            else if(file.find("specular") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.specular = std::move(temp);
            }
            else if(file.find("height") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.height = std::move(temp);
            }
            else if(file.find("ambient_occlusion") != std::string::npos) {
                temp.createTextureImage(file, device, commandbuffer, framebuffer, sampler, false, false);
                temp.createTextureImageView(swapchain, device.getDevice(), false);
                temp.createTextureSampler(device.getDevice(), device.getPhysicalDevice(), false);

                scene.ambientOcclusion = std::move(temp);
            } else {
                std::cout << "Textures found but naming convention not followed" << std::endl;
                // manually add textures
                // clear texture memory
            }

            temp.cleanup(device.getDevice());
        }
    }

    scene.totalTextures = texturePaths.size();

    scenes.push_back(scene);
}

void Engine::Core::RT::SceneManager::remove(int index)
{
    scenes.erase(scenes.begin() + index);
    raytrace.sceneUpdated = true;
}

void Engine::Core::RT::SceneManager::pushToAccelerationStructure(std::vector<RTScene>& dst) {
    for(RTScene scene : scenes) {
        dst.push_back(scene);
    }
}

void Engine::Core::RT::SceneManager::updateScene() {
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

    for (int i = 0; i < scenes.size(); i++) {
        if (scenes[i].markedForDeletion) {
            remove(i);
        }
    }

    int i = 0;
    bool transformChanged = false;
    for(RTScene& scene : scenes) {
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Text("%s", scene.name.c_str());

        glm::mat4& matrix = scene.matrix;

        static ImGuizmo::OPERATION currOp = ImGuizmo::TRANSLATE;
        static ImGuizmo::MODE currMode = ImGuizmo::WORLD;

        ImGui::Checkbox("Show Gizmo", &scene.showGizmo);

        if (scene.showGizmo) {
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

        if (ImGui::Button("Remove Entity"))
            scene.markedForDeletion = true;

        ImGui::PopID();

        i++;
    }

    if (transformChanged) {
        for (int i = 0; i < scenes.size(); i++) {
            raytrace.models[i].matrix = scenes[i].matrix;
        }
        raytrace.updateTopLevelAccelerationStructure(device, framebuffer, commandbuffer, true);
        raytrace.uboData.sampleCount = 1;
    }
}