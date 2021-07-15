#include <serenity/editor/windowScene/WindowScene.h>
#include <serenity/editor/imgui/imgui.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/ecs/components/Components.h>
#include <serenity/system/Engine.h>
#include <serenity/math/MathCompression.h>

#include <serenity/lights/Lights.h>

#include <serenity/renderer/postprocess/HDR.h>
#include <serenity/renderer/postprocess/SSAO.h>
#include <serenity/renderer/postprocess/SMAA.h>
#include <serenity/renderer/postprocess/FXAA.h>
#include <serenity/renderer/postprocess/Bloom.h>

#include <serenity/networking/Networking.h>

#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#endif

void Engine::priv::EditorWindowScene::internal_render_network() {
    const ImVec4 yellow      = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const auto& tcpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_TCPSockets;
    const auto& tcpListeners = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_TCPListeners;
    const auto& udpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_UDPSockets;
    ImGui::TextColored(yellow, ("Number of TCP sockets:   " + std::to_string(tcpSockets.size())).c_str());
    ImGui::TextColored(yellow, ("Number of TCP listeners: " + std::to_string(tcpListeners.size())).c_str());
    ImGui::TextColored(yellow, ("Number of UDP sockets:   " + std::to_string(udpSockets.size())).c_str());

    ImGui::BeginChild("NetworkSockets");
    if (tcpSockets.size() > 0) {
        if (ImGui::TreeNode("TCP Sockets")) {
            for (const auto& tcpSocket : tcpSockets) {
                ImGui::Text(("Ip:          " + tcpSocket->ip().toString()).c_str());
                ImGui::Text(("Connected:   " + std::string(tcpSocket->isConnected() ? "true" : "false")).c_str());
                ImGui::Text(("Blocking:    " + std::string(tcpSocket->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(("Local Port:  " + std::to_string(tcpSocket->localPort())).c_str());
                ImGui::Text(("Remote Port: " + std::to_string(tcpSocket->remotePort())).c_str());

                ImGui::Separator();
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    if (tcpListeners.size() > 0) {
        if (ImGui::TreeNode("TCP Listeners")) {
            for (const auto& tcpListener : tcpListeners) {
                ImGui::Text(("Listening:  " + std::string(tcpListener->isListening() ? "true" : "false")).c_str());
                ImGui::Text(("Blocking:   " + std::string(tcpListener->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(("Local Port: " + std::to_string(tcpListener->localPort())).c_str());

                ImGui::Separator();
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    if (udpSockets.size() > 0) {
        if (ImGui::TreeNode("UDP Sockets")) {
            for (const auto& udpSocket : udpSockets) {
                ImGui::Text(("Ip:                  " + udpSocket->m_IP.toString()).c_str());
                ImGui::Text(("Bound:               " + std::string(udpSocket->isBound() ? "true" : "false")).c_str());
                ImGui::Text(("Blocking:            " + std::string(udpSocket->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(("Local Port:          " + std::to_string(udpSocket->localPort())).c_str());
                ImGui::Text(("Num Partial Packets: " + std::to_string(udpSocket->getNumPartialPackets())).c_str());
                if (udpSocket->getNumPartialPackets() > 0) {
                    if (ImGui::TreeNode("Partial Packets")) {
                        for (const auto& packetInfo : udpSocket->m_PartialPackets) {
                            ImGui::Text(("Valid:           " + std::string(packetInfo.packet.m_Valid ? "true" : "false")).c_str());
                            ImGui::Text(("Timestamp:       " + std::to_string(packetInfo.packet.m_Timestamp)).c_str());
                            ImGui::Text(("Type:            " + std::to_string(packetInfo.packet.m_PacketType)).c_str());
                            ImGui::Text(("Ack:             " + std::to_string(packetInfo.packet.m_Ack)).c_str());
                            ImGui::Text(("Ack Bitfield:    " + std::to_string(packetInfo.packet.m_AckBitfield)).c_str());
                            ImGui::Text(("Sequence Number: " + std::to_string(packetInfo.packet.m_SequenceNumber)).c_str());
                            ImGui::Text(("Data Size Bytes: " + std::to_string(packetInfo.packet.getDataSize())).c_str());
                        }
                        ImGui::TreePop();
                        ImGui::Separator();
                    }
                }
                ImGui::Separator();
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
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
                auto rigid     = e.getComponent<ComponentRigidBody>();
                auto shape     = e.getComponent<ComponentCollisionShape>();
                auto model     = e.getComponent<ComponentModel>();
                auto cam       = e.getComponent<ComponentCamera>();
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
                        if (ImGui::TreeNode(("ModelInstance " + std::to_string(i)).c_str())) {
                            ModelInstance& instance = model->getModel(i);
                            const auto& color       = instance.getColor();
                            const auto& GRColor     = instance.getGodRaysColor();
                            float aColor[4]         = { color.r(), color.g(), color.b(), color.a() };
                            float aGodRays[3]       = { GRColor.r(), GRColor.g(), GRColor.b() };
                            ImGui::ColorEdit4("Color", &aColor[0]);
                            ImGui::ColorEdit3("God Rays Color", &aGodRays[0]);
                            ImGui::Checkbox("Force Render", &instance.m_ForceRender);
                            ImGui::Checkbox("Cast Shadow", &instance.m_IsShadowCaster);
                            ImGui::Checkbox("Show", &instance.m_Visible);
                            ImGui::Separator();
                            ImGui::InputFloat3("position", &instance.m_Position[0]);
                            ImGui::InputFloat4("rotation", &instance.m_Orientation[0]);
                            ImGui::InputFloat3("scale", &instance.m_Scale[0]);

                            instance.setColor(aColor[0], aColor[1], aColor[2], aColor[3]);
                            instance.setGodRaysColor(aGodRays[0], aGodRays[1], aGodRays[2]);
                            instance.internal_update_model_matrix(false);

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
                    } else {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Orthographic Camera");
                        ImGui::InputFloat("Left", &cam->m_Left);
                        ImGui::InputFloat("Right", &cam->m_Right);
                        ImGui::InputFloat("Top", &cam->m_Top);
                        ImGui::InputFloat("Bottom", &cam->m_Bottom);
                        ImGui::InputFloat("Near", &cam->m_NearPlane);
                        ImGui::InputFloat("Far", &cam->m_FarPlane);
                    }
                    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*cam);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
        ImGui::Separator();
    }
    
    auto& directionalLights = Engine::priv::PublicScene::GetLights<DirectionalLight>(currentScene);
    auto& sunLights         = Engine::priv::PublicScene::GetLights<SunLight>(currentScene);
    auto& pointLights       = Engine::priv::PublicScene::GetLights<PointLight>(currentScene);
    auto& spotLights        = Engine::priv::PublicScene::GetLights<SpotLight>(currentScene);
    auto& rodLights         = Engine::priv::PublicScene::GetLights<RodLight>(currentScene);

    auto base_light_logic = [](auto& light) {
        bool isLightActive = light.isActive();
        ImGui::Checkbox("Enabled", &isLightActive);
        light.activate(isLightActive);

        bool isShadowCaster = light.isShadowCaster();
        ImGui::Checkbox("Casts Shadows", &isShadowCaster);
        light.setShadowCaster(isShadowCaster);

        const auto& color = light.getColor();
        ImGui::ColorEdit4("Color", const_cast<float*>(glm::value_ptr(color)));
        light.setColor(color);

        auto diff = light.getDiffuseIntensity();
        auto spec = light.getSpecularIntensity();
        ImGui::SliderFloat("Diffuse Intensity", &diff, 0.0f, 20.0f);
        ImGui::SliderFloat("Specular Intensity", &spec, 0.0f, 20.0f);
        light.setDiffuseIntensity(diff);
        light.setSpecularIntensity(spec);

        //TODO: position? all lights use position EXCEPT directional lights
    };

    auto lamda_lights = [&](auto& container, const char* nodeName, const char* lightName, auto functor) {
        if (container.size() > 0) {
            if (ImGui::TreeNode(nodeName)) {
                for (int i = 0; i < container.size(); ++i) {
                    const std::string title = (std::string(lightName) + " " + std::to_string(i));
                    if (ImGui::TreeNode(title.c_str())) {
                        base_light_logic(*container[i]);
                        functor(*container[i]);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
                ImGui::Separator();
            }
        }
    };

    if (directionalLights.size() > 0 || sunLights.size() > 0 || pointLights.size() > 0 || spotLights.size() > 0 || rodLights.size() > 0) {
        if (ImGui::TreeNode("Lights")) {
            lamda_lights(directionalLights, "Directional Lights", "Directional Light", [&](DirectionalLight& light) {
                const auto dir = light.getDirection();
                ImGui::InputFloat3("Direction", const_cast<float*>(glm::value_ptr(dir)));
                light.setDirection(dir); //TODO: add a check to see if the direction is different in this function call? this is quite expensive to do per frame
            });
            lamda_lights(sunLights, "Sun Lights", "Sun Light", [&](SunLight& light) {
                auto transform   = light.getComponent<ComponentTransform>();
                ImGui::InputDouble3("Position", &transform->m_Position[0]);
            });
            lamda_lights(pointLights, "Point Lights", "Point Light", [&](PointLight& light) {
                auto transform   = light.getComponent<ComponentTransform>();
                ImGui::InputDouble3("Position", &transform->m_Position[0]);
                auto constant_   = light.getConstant();
                auto linear_     = light.getLinear();
                auto exponent_   = light.getExponent();
                ImGui::SliderFloat("Constant", &constant_, 0.0f, 5.0f);
                ImGui::SliderFloat("Linear", &linear_, 0.0f, 5.0f);
                ImGui::SliderFloat("Exponent", &exponent_, 0.0f, 5.0f);
                light.setConstant(constant_);
                light.setLinear(linear_);
                light.setExponent(exponent_);
                //TODO: add attenuation model
            });
            lamda_lights(spotLights, "Spot Lights", "Spot Light", [&](SpotLight& light) {
                auto transform   = light.getComponent<ComponentTransform>();
                ImGui::InputDouble3("Position", &transform->m_Position[0]);

                auto cutoff      = light.getCutoff();
                auto outerCutoff = light.getCutoffOuter();
                ImGui::SliderFloat("Inner Cutoff", &cutoff, 0.0f, 360.0f);
                ImGui::SliderFloat("Outer Cutoff", &outerCutoff, 0.0f, 360.0f);
                light.setCutoffDegrees(cutoff);
                light.setCutoffOuterDegrees(outerCutoff);
            });
            lamda_lights(rodLights, "Rod Lights", "Rod Light", [&](RodLight& light) {
                auto transform   = light.getComponent<ComponentTransform>();
                ImGui::InputDouble3("Position", &transform->m_Position[0]);
            });
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
}
void Engine::priv::EditorWindowScene::internal_render_profiler() {
    const auto& debugging = Engine::priv::Core::m_Engine->m_DebugManager;
    const ImVec4 yellow   = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
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
    DWORDLONG virtualMemUsed  = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile; //Virtual Memory currently used
    DWORDLONG totalPhysMem    = memInfo.ullTotalPhys; //Total Physical Memory (RAM)
    DWORDLONG physMemUsed     = memInfo.ullTotalPhys - memInfo.ullAvailPhys; //Physical Memory currently used

    ImGui::TextColored(yellow, ("Total Virtual Memory: " + byte_format(totalVirtualMem)).c_str());
    ImGui::TextColored(yellow, ("Virtual Memory Used: " + byte_format(virtualMemUsed)).c_str());
    ImGui::TextColored(yellow, ("Total Physical Memory (RAM): " + byte_format(totalPhysMem)).c_str());
    ImGui::TextColored(yellow, ("Physical Memory Used: " + byte_format(physMemUsed)).c_str());

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage; //Virtual Memory currently used by current process
    SIZE_T physMemUsedByMe    = pmc.WorkingSetSize; //Physical Memory currently used by current process
    ImGui::TextColored(yellow, ("Virtual Memory used by current process: " + byte_format(virtualMemUsedByMe)).c_str());
    ImGui::TextColored(yellow, ("Physical Memory used by current process: " + byte_format(physMemUsedByMe)).c_str());
#endif

}
void Engine::priv::EditorWindowScene::internal_render_renderer() {
    auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
    //general
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "General");
        static bool vsync = false;
        ImGui::Checkbox("Vsync ", &vsync);
        Engine::Resources::getWindow().setVerticalSyncEnabled(vsync);
    }
    //skybox
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Skybox");
        ImGui::Checkbox("Skybox Enabled ", &renderer.m_DrawSkybox);
    }
    //lighting
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Lighting");
        ImGui::Checkbox("Lighting Enabled", &renderer.m_Lighting);
        ImGui::SliderFloat("GI Contribution Diffuse", &renderer.m_GI_Diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("GI Contribution Specular", &renderer.m_GI_Specular, 0.0f, 1.0f);
        ImGui::SliderFloat("GI Contribution Global", &renderer.m_GI_Global, 0.0f, 1.0f);
        renderer.m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(renderer.m_GI_Diffuse, renderer.m_GI_Specular, renderer.m_GI_Global);
        static const char* LightingModels[] = { "Basic", "Physical" };
        static int lighting_model_current   = int(renderer.m_LightingAlgorithm);
        ImGui::ListBox("Lighting Model", &lighting_model_current, LightingModels, IM_ARRAYSIZE(LightingModels));
        if (lighting_model_current == int(LightingAlgorithm::Basic)) {
            auto scene = Engine::Resources::getCurrentScene();
            if (scene) {
                const auto& ambientColor      = Engine::Resources::getCurrentScene()->getAmbientColor();
                static float ambientColors[3] = { ambientColor.r, ambientColor.g, ambientColor.b };
                ImGui::ColorEdit3("Ambient Color", &ambientColors[0]);
                scene->setAmbientColor(ambientColors[0], ambientColors[1], ambientColors[2]);
            }
        }
        Engine::Renderer::Settings::Lighting::setLightingAlgorithm(static_cast<LightingAlgorithm>(lighting_model_current));
        ImGui::Separator();
    }
    //hdr
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "HDR");
        static const char* HDRAlgos[] = { "None", "Reinhard", "Filmic", "Exposure", "Uncharted" };
        static int hdr_algo_current   = int(Engine::priv::HDR::STATIC_HDR.m_Algorithm);
        ImGui::ListBox("HDR Algorithm", &hdr_algo_current, HDRAlgos, IM_ARRAYSIZE(HDRAlgos));
        ImGui::SliderFloat("HDR Exposure", &Engine::priv::HDR::STATIC_HDR.m_Exposure, -15.0f, 15.0f);
        Engine::Renderer::hdr::setAlgorithm(static_cast<HDRAlgorithm::Algorithm>(hdr_algo_current));
        ImGui::Separator();
    }
    //bloom
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Bloom");
        static bool bloom_enabled = Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Active;
        ImGui::Checkbox("Bloom Enabled", &bloom_enabled);
        ImGui::SliderFloat("Bloom Exposure", &Engine::priv::Bloom::STATIC_BLOOM.m_Exposure, -5.0f, 5.0f);
        ImGui::SliderFloat("Bloom Threshold", &Engine::priv::Bloom::STATIC_BLOOM.m_Threshold, -5.0f, 5.0f);
        ImGui::SliderFloat("Bloom Scale", &Engine::priv::Bloom::STATIC_BLOOM.m_Scale, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Blur Radius", &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Radius, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Blur Strength", &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Strength, -5.0f, 5.0f);
        static int bloom_samples = Engine::priv::Bloom::STATIC_BLOOM.m_Num_Passes;
        ImGui::SliderInt("Bloom Num Passes", &bloom_samples, 0, 8);
        Engine::Renderer::bloom::enable(bloom_enabled);
        Engine::Renderer::bloom::setNumPasses(uint32_t(bloom_samples));
        ImGui::Separator();
    }
    //ssao
    {
        static const char* SSAOLevels[] = { "Off", "Low", "Medium", "High", "Ultra" };
        static int ssao_algo_current    = int(Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel);
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Screen Space Ambient Occlusion");
        ImGui::ListBox("SSAO Level", &ssao_algo_current, SSAOLevels, IM_ARRAYSIZE(SSAOLevels));
        Engine::Renderer::ssao::setLevel((SSAOLevel::Level)ssao_algo_current);
        ImGui::SliderFloat("SSAO Bias", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_bias, -3.0f, 3.0f);
        ImGui::SliderFloat("SSAO Scale", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_scale, 0.0f, 3.0f);
        ImGui::SliderFloat("SSAO Radius", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_radius, 0.0f, 10.0f);
        ImGui::SliderFloat("SSAO Intensity", &Engine::priv::SSAO::STATIC_SSAO.m_ssao_intensity, 0.0f, 10.0f);
        static int ssao_samples = Engine::priv::SSAO::STATIC_SSAO.m_ssao_samples;
        ImGui::SliderInt("SSAO Samples", &ssao_samples, 0, int(SSAO_MAX_KERNEL_SIZE));
        Engine::Renderer::ssao::setSamples(static_cast<uint32_t>(ssao_samples));
        ImGui::Separator();
    }
    //anti-aliasing
    {
        static const char* AAAlgos[] = { "Off", "FXAA", "SMAA Low", "SMAA Medium", "SMAA High", "SMAA Ultra" };
        static int aa_algo_current   = int(renderer.m_AA_algorithm);
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Anti-Aliasing");
        ImGui::ListBox("Anti-Aliasing Algorithm", &aa_algo_current, AAAlgos, IM_ARRAYSIZE(AAAlgos));
        Engine::Renderer::Settings::setAntiAliasingAlgorithm((AntiAliasingAlgorithm)aa_algo_current);
        ImGui::Separator();
    }
    //debugging
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Debugging");
        ImGui::Checkbox("Draw Physics", &renderer.m_DrawPhysicsDebug);
        ImGui::Separator();
    }
}
void Engine::priv::EditorWindowScene::update() {
    auto currScene        = Engine::Resources::getCurrentScene();
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
        if (ImGui::BeginTabItem("Network")) {
            m_Tab = 3;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleColor();
    switch (m_Tab) {
        case 0: { //Entities
            internal_render_entities(*currScene);
            break;
        } case 1: { //Renderer
            internal_render_renderer();
            break;
        } case 2: { //Profiler
            internal_render_profiler();
            break;
        } case 3: { //Network
            internal_render_network();
            break;
        }
    }

    // Plot some values
    //const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
    //ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

    ImGui::End();
}