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