#include <serenity/editor/windowScene/WindowScene.h>
#include <serenity/editor/imgui/imgui.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/ecs/components/Components.h>
#include <serenity/system/Engine.h>
#include <serenity/math/MathCompression.h>

#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/postprocess/SMAA.h>
#include <serenity/renderer/postprocess/FXAA.h>
#include <serenity/renderer/postprocess/Bloom.h>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#endif

Engine::priv::EditorWindowScene::EditorWindowScene() {

}

Engine::priv::EditorWindowScene::~EditorWindowScene() {
}
void Engine::priv::EditorWindowScene::internal_render_entities(Scene& currentScene) {
    ImGui::BeginChild("SceneEntities");
    const auto& entities = Engine::priv::PublicScene::GetEntities(currentScene);
    if (ImGui::TreeNode("Entities")) {
        for (const auto e : entities) {
            auto name = e.getComponent<ComponentName>();
            if (ImGui::TreeNode(("Entity " + std::to_string(e.id()) + (name ? (" - " + name->name()) : "")).c_str())) {
                //for each component...
                auto transform = e.getComponent<ComponentTransform>();
                auto rigid = e.getComponent<ComponentRigidBody>();
                auto shape = e.getComponent<ComponentCollisionShape>();
                auto model = e.getComponent<ComponentModel>();
                auto cam = e.getComponent<ComponentCamera>();
                if (name && ImGui::TreeNode("ComponentName")) {
                    ImGui::TreePop();
                }
                if (transform && ImGui::TreeNode("ComponentTransform")) {
                    ImGui::InputDouble3("position", &transform->m_Position[0]);
                    ImGui::InputFloat4("rotation", &transform->m_Rotation[0]);
                    ImGui::InputFloat3("scale", &transform->m_Scale[0]);

                    //getChildren() is somewhat expensive, 0(N) N = num max entities in scene
                    const auto& children = e.getChildren();
                    if (children.size() > 0) {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Children:");
                        for (const auto child : children) {
                            ImGui::Text(("Entity " + std::to_string(child.id())).c_str());
                        }
                        ImGui::Separator();
                    }
                    ImGui::TreePop();
                }
                if (rigid && ImGui::TreeNode("ComponentRigidBody")) {
                    ImGui::TreePop();
                }
                if (shape && ImGui::TreeNode("ComponentCollisionShape")) {
                    ImGui::TreePop();
                }
                if (model && ImGui::TreeNode("ComponentModel")) {
                    for (size_t i = 0; i < model->getNumModels(); ++i) {
                        ModelInstance& instance = model->getModel(i);
                        if (ImGui::TreeNode(("ModelInstance " + std::to_string(i)).c_str())) {
                            const auto& color = instance.getColor();
                            const auto& GRColor = instance.getGodRaysColor();
                            float aColor[4] = { color.r(), color.g(), color.b(), color.a() };
                            float aGodRays[3] = { GRColor.r(), GRColor.g(), GRColor.b() };
                            ImGui::ColorEdit4("Color", &aColor[0]);
                            ImGui::ColorEdit3("God Rays Color", &aGodRays[0]);
                            ImGui::Checkbox("Force Render", &instance.m_ForceRender);
                            ImGui::Separator();
                            ImGui::InputFloat3("position", &instance.m_Position[0]);
                            ImGui::InputFloat4("rotation", &instance.m_Orientation[0]);
                            ImGui::InputFloat3("scale", &instance.m_Scale[0]);

                            instance.setColor(aColor[0], aColor[1], aColor[2], aColor[3]);
                            instance.setGodRaysColor(aGodRays[0], aGodRays[1], aGodRays[2]);
                            instance.internal_update_model_matrix(true);

                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (cam && ImGui::TreeNode("ComponentCamera")) {
                    if (cam->getType() == ComponentCamera::CameraType::Perspective) {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Perspective Camera");
                        ImGui::SliderFloat("Angle", &cam->m_Angle, 0.0f, glm::radians(180.0f));
                        ImGui::InputFloat("AspectRatio", &cam->m_AspectRatio);
                        ImGui::InputFloat("Near", &cam->m_NearPlane);
                        ImGui::InputFloat("Far", &cam->m_FarPlane);
                    }else{
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Orthographic Camera");
                        ImGui::InputFloat("Left", &cam->m_Left);
                        ImGui::InputFloat("Right", &cam->m_Right);
                        ImGui::InputFloat("Top", &cam->m_Top);
                        ImGui::InputFloat("Bottom", &cam->m_Bottom);
                        ImGui::InputFloat("Near", &cam->m_NearPlane);
                        ImGui::InputFloat("Far", &cam->m_FarPlane);
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
        ImGui::Separator();
    }
    ImGui::EndChild();
}
void Engine::priv::EditorWindowScene::internal_render_profiler() {
    const auto& debugging = Engine::priv::Core::m_Engine->m_DebugManager;
    const ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::TextColored(yellow, ("Update Time:  " + debugging.updateTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, ("Physics Time: " + debugging.physicsTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, ("Sounds Time:  " + debugging.soundsTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, ("Render Time:  " + debugging.renderTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, ("Delta Time:   " + debugging.deltaTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, ("FPS:  " + debugging.fps()).c_str());

#ifdef _WIN32

    auto byte_format = [](DWORDLONG input) {
        std::stringstream strm;
        if (input >= 1099511627776) {
            strm << std::fixed << std::setprecision(4) << ((double)input / 1099511627776.0) << " TB";
        }else if (input >= 1073741824) {
            strm << std::fixed << std::setprecision(4) << ((double)input / 1073741824.0) << " GB";
        }else if (input >= 1048576) {
            strm << std::fixed << std::setprecision(4) << ((double)input / 1048576.0) << " MB";
        }else if (input >= 1024) {
            strm << std::fixed << std::setprecision(4) << ((double)input / 1024.0) << " KB";
        }else{
            strm << input << " bytes";
        }
        return strm.str();
    };

    ImGui::Separator();
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile; //Total Virtual Memory
    DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile; //Virtual Memory currently used
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys; //Total Physical Memory (RAM)
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys; //Physical Memory currently used

    ImGui::TextColored(yellow, ("Total Virtual Memory: " + byte_format(totalVirtualMem)).c_str());
    ImGui::TextColored(yellow, ("Virtual Memory Used: " + byte_format(virtualMemUsed)).c_str());
    ImGui::TextColored(yellow, ("Total Physical Memory (RAM): " + byte_format(totalPhysMem)).c_str());
    ImGui::TextColored(yellow, ("Physical Memory Used: " + byte_format(physMemUsed)).c_str());

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage; //Virtual Memory currently used by current process
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize; //Physical Memory currently used by current process
    ImGui::TextColored(yellow, ("Virtual Memory used by current process: " + byte_format(virtualMemUsedByMe)).c_str());
    ImGui::TextColored(yellow, ("Physical Memory used by current process: " + byte_format(physMemUsedByMe)).c_str());
#endif

}
void Engine::priv::EditorWindowScene::internal_render_renderer() {
    auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;

    //lighting
    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Lighting");
    ImGui::Checkbox("Enabled", &renderer.m_Lighting);
    ImGui::SliderFloat("GI Contribution Diffuse", &renderer.m_GI_Diffuse, 0.0f, 1.0f);
    ImGui::SliderFloat("GI Contribution Specular", &renderer.m_GI_Specular, 0.0f, 1.0f);
    ImGui::SliderFloat("GI Contribution Global", &renderer.m_GI_Global, 0.0f, 1.0f);
    renderer.m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(renderer.m_GI_Diffuse, renderer.m_GI_Specular, renderer.m_GI_Global);
    const char* LightingModels[] = { "Basic", "Physical" };
    static int lighting_model_current = (int)renderer.m_LightingAlgorithm;
    ImGui::ListBox("Lighting Model", &lighting_model_current, LightingModels, IM_ARRAYSIZE(LightingModels));
    ImGui::Separator();

    //hdr
    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "HDR");
    const char* HDRAlgos[] = { "None", "Reinhard", "Filmic", "Exposure", "Uncharted" };
    static int hdr_algo_current = (int)Engine::priv::HDR::STATIC_HDR.m_Algorithm;
    ImGui::ListBox("HDR Algorithm", &hdr_algo_current, HDRAlgos, IM_ARRAYSIZE(HDRAlgos));
    ImGui::SliderFloat("Exposure", &Engine::priv::HDR::STATIC_HDR.m_Exposure, -15.0f, 15.0f);
    ImGui::Separator();

    //ssao
    const char* SSAOLevels[] = { "Off", "Low", "Medium", "High", "Ultra" };
    static int ssao_algo_current = (int)Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel;
    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Screen Space Ambient Occlusion");
    ImGui::ListBox("SSAO Level", &ssao_algo_current, SSAOLevels, IM_ARRAYSIZE(SSAOLevels));
    Engine::Renderer::ssao::setLevel((SSAOLevel::Level)ssao_algo_current);
    ImGui::SliderFloat("Bias", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias, -3.0f, 3.0f);
    ImGui::SliderFloat("Scale", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale, 0.0f, 3.0f);
    ImGui::SliderFloat("Radius", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius, 0.0f, 10.0f);
    ImGui::SliderFloat("Intensity", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity, 0.0f, 10.0f);
    int samples = Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples;
    ImGui::SliderInt("Samples", &samples, 0, (int)SSAO_MAX_KERNEL_SIZE);
    ImGui::Separator();

    //anti-aliasing
    const char* AAAlgos[] = { "Off", "FXAA", "SMAA Low", "SMAA Medium", "SMAA High", "SMAA Ultra" };
    static int aa_algo_current = (int)renderer.m_AA_algorithm;
    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Anti-Aliasing");
    ImGui::ListBox("Anti-Aliasing Algorithm", &aa_algo_current, AAAlgos, IM_ARRAYSIZE(AAAlgos));
    Engine::Renderer::Settings::setAntiAliasingAlgorithm((AntiAliasingAlgorithm)aa_algo_current);
    ImGui::Separator();

    //debugging
    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Debugging");
    ImGui::Checkbox("Draw Physics", &renderer.m_DrawPhysicsDebug);
    ImGui::Separator();

    Engine::Renderer::hdr::setAlgorithm(static_cast<HDRAlgorithm::Algorithm>(hdr_algo_current));
    Engine::Renderer::Settings::Lighting::setLightingAlgorithm(static_cast<LightingAlgorithm>(lighting_model_current));
    Engine::Renderer::ssao::setSamples(static_cast<uint32_t>(samples));
}
void Engine::priv::EditorWindowScene::update() {
    auto currScene = Engine::Resources::getCurrentScene();
    std::string sceneName = currScene ? currScene->name() : "N/A";
    ImGui::Begin(("Scene - " + sceneName).c_str(), NULL);
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    if (ImGui::BeginTabBar("TabBar")) {
        if (ImGui::BeginTabItem("Entities")) {
            m_Tab = 0;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Renderer")) {
            m_Tab = 1;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Profiler")) {
            m_Tab = 2;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleColor();
    switch (m_Tab) {
        case 0: { //Entities
            internal_render_entities(*currScene);
            break;
        }
        case 1: { //Renderer
            internal_render_renderer();
            break;
        }
        case 2: { //Profiler
            internal_render_profiler();
            break;
        }
    }

    // Plot some values
    //const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
    //ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

    ImGui::End();
}