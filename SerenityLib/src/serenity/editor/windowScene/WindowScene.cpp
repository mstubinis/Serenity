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

#include <serenity/resources/material/Material.h>
#include <serenity/resources/shader/ShaderProgram.h>

#include <serenity/networking/Networking.h>

#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#endif

using InternalFunc = void(Engine::priv::EditorWindowScene::*)(Scene&);
namespace Engine::priv {
    class EditorWindowSceneFunctions {
        public: 
            constexpr static std::array<std::tuple<const char*, InternalFunc>, (size_t)Engine::priv::EditorWindowScene::TabType::_TOTAL> TAB_TYPES_DATA{ {
                { "Entities", &Engine::priv::EditorWindowScene::internal_render_entities },
                { "Renderer", &Engine::priv::EditorWindowScene::internal_render_renderer },
                { "Resources", &Engine::priv::EditorWindowScene::internal_render_resources },
                { "Profiler", &Engine::priv::EditorWindowScene::internal_render_profiler },
                { "Network", &Engine::priv::EditorWindowScene::internal_render_network },
            } };
    };
}


void Engine::priv::EditorWindowScene::internal_render_network(Scene& currentScene) {
    const ImVec4 yellow      = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const auto& tcpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_TCPSockets;
    const auto& tcpListeners = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_TCPListeners;
    const auto& udpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.m_SocketManager.m_UDPSockets;
    ImGui::TextColored(yellow, std::string("Number of TCP sockets:   " + std::to_string(tcpSockets.size())).c_str());
    ImGui::TextColored(yellow, std::string("Number of TCP listeners: " + std::to_string(tcpListeners.size())).c_str());
    ImGui::TextColored(yellow, std::string("Number of UDP sockets:   " + std::to_string(udpSockets.size())).c_str());

    ImGui::BeginChild("NetworkSockets");
    if (tcpSockets.size() > 0) {
        if (ImGui::TreeNode("TCP Sockets")) {
            for (const auto& tcpSocket : tcpSockets) {
                ImGui::Text(std::string("Ip:          " + tcpSocket->ip().toString()).c_str());
                ImGui::Text(std::string("Connected:   " + std::string(tcpSocket->isConnected() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Blocking:    " + std::string(tcpSocket->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Local Port:  " + std::to_string(tcpSocket->localPort())).c_str());
                ImGui::Text(std::string("Remote Port: " + std::to_string(tcpSocket->remotePort())).c_str());

                ImGui::Separator();
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    if (tcpListeners.size() > 0) {
        if (ImGui::TreeNode("TCP Listeners")) {
            for (const auto& tcpListener : tcpListeners) {
                ImGui::Text(std::string("Listening:  " + std::string(tcpListener->isListening() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Blocking:   " + std::string(tcpListener->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Local Port: " + std::to_string(tcpListener->localPort())).c_str());

                ImGui::Separator();
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    if (udpSockets.size() > 0) {
        if (ImGui::TreeNode("UDP Sockets")) {
            for (const auto& udpSocket : udpSockets) {
                ImGui::Text(std::string("Ip:                  " + udpSocket->m_IP.toString()).c_str());
                ImGui::Text(std::string("Bound:               " + std::string(udpSocket->isBound() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Blocking:            " + std::string(udpSocket->isBlocking() ? "true" : "false")).c_str());
                ImGui::Text(std::string("Local Port:          " + std::to_string(udpSocket->localPort())).c_str());
                ImGui::Text(std::string("Num Partial Packets: " + std::to_string(udpSocket->getNumPartialPackets())).c_str());
                if (udpSocket->getNumPartialPackets() > 0) {
                    if (ImGui::TreeNode("Partial Packets")) {
                        for (const auto& packetInfo : udpSocket->m_PartialPackets) {
                            ImGui::Text(std::string("Valid:           " + std::string(packetInfo.packet.m_Valid ? "true" : "false")).c_str());
                            ImGui::Text(std::string("Timestamp:       " + std::to_string(packetInfo.packet.m_Timestamp)).c_str());
                            ImGui::Text(std::string("Type:            " + std::to_string(packetInfo.packet.m_PacketType)).c_str());
                            ImGui::Text(std::string("Ack:             " + std::to_string(packetInfo.packet.m_Ack)).c_str());
                            ImGui::Text(std::string("Ack Bitfield:    " + std::to_string(packetInfo.packet.m_AckBitfield)).c_str());
                            ImGui::Text(std::string("Sequence Number: " + std::to_string(packetInfo.packet.m_SequenceNumber)).c_str());
                            ImGui::Text(std::string("Data Size Bytes: " + std::to_string(packetInfo.packet.getDataSize())).c_str());
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
                auto camera    = e.getComponent<ComponentCamera>();
                auto script    = e.getComponent<ComponentScript>();
                if (name && ImGui::TreeNode("ComponentName")) {
                    ImGui::Text(std::string("Name: " + name->name()).c_str());
                    ImGui::TreePop();
                }
                if (transform && ImGui::TreeNode("ComponentTransform")) {
                    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Position"); ImGui::SameLine(); ImGui::InputDouble3("##pos", &transform->m_Position[0]);
                    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Rotation"); ImGui::SameLine(); ImGui::InputFloat4("##rot", &transform->m_Rotation[0]);
                    ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Scale   "); ImGui::SameLine(); ImGui::InputFloat3("##scl", &transform->m_Scale[0]);

                    //getChildren() is somewhat expensive, 0(N) N = num max entities in scene
                    const auto& children = e.getChildren();
                    if (children.size() > 0) {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Children:");
                        for (const auto child : children) {
                            ImGui::Text(std::string("Entity: " + child.toString()).c_str());
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
                            const auto& color    = instance.getColor();
                            const auto& GRColor  = instance.getGodRaysColor();
                            float aColor[4]      = { color.r(), color.g(), color.b(), color.a() };
                            float aGodRays[3]    = { GRColor.r(), GRColor.g(), GRColor.b() };
                            ImGui::ColorEdit4("Color", &aColor[0]);
                            ImGui::ColorEdit3("God Rays Color", &aGodRays[0]);

                            ImGui::Checkbox("Force Render", &instance.m_ForceRender);
                            ImGui::SameLine();
                            ImGui::Checkbox("Cast Shadow", &instance.m_IsShadowCaster);
                            ImGui::SameLine();
                            ImGui::Checkbox("Show", &instance.m_Visible);
                            ImGui::SameLine();
                            
                            ImGui::Separator();
                            ImGui::InputFloat3("position", &instance.m_Position[0]);
                            ImGui::InputFloat4("rotation", &instance.m_Orientation[0]);
                            ImGui::InputFloat3("scale", &instance.m_Scale[0]);
                            ImGui::Text(std::string("Radius: " + std::to_string(instance.m_Radius)).c_str());
                            ImGui::Separator();
                            const std::string a = instance.m_MeshHandle.null() ? "N/A" : instance.m_MeshHandle.get<Mesh>()->name();
                            const std::string b = instance.m_MaterialHandle.null() ? "N/A" : instance.m_MaterialHandle.get<Material>()->name();
                            const std::string c = instance.m_ShaderProgramHandle.null() ? "N/A" : instance.m_ShaderProgramHandle.get<ShaderProgram>()->name();
                            ImGui::Text(std::string("Mesh: " + a).c_str());
                            ImGui::Text(std::string("Material: " + b).c_str());
                            ImGui::Text(std::string("Shader: " + c).c_str());
                            ImGui::Text(std::string("Stage: " + std::string(instance.m_Stage.toString())).c_str());
                            ImGui::Separator();
                            instance.setColor(aColor[0], aColor[1], aColor[2], aColor[3]);
                            instance.setGodRaysColor(aGodRays[0], aGodRays[1], aGodRays[2]);
                            instance.internal_update_model_matrix(false);

                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (camera && ImGui::TreeNode("ComponentCamera")) {
                    if (camera->getType() == ComponentCamera::CameraType::Perspective) {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Perspective Camera");
                        ImGui::SliderFloat("Angle", &camera->m_Angle, 0.0f, glm::radians(180.0f));
                        ImGui::InputFloat("AspectRatio", &camera->m_AspectRatio);
                        ImGui::InputFloat("Near", &camera->m_NearPlane);
                        ImGui::InputFloat("Far", &camera->m_FarPlane);
                    } else {
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Orthographic Camera");
                        ImGui::InputFloat("Left", &camera->m_Left);
                        ImGui::InputFloat("Right", &camera->m_Right);
                        ImGui::InputFloat("Top", &camera->m_Top);
                        ImGui::InputFloat("Bottom", &camera->m_Bottom);
                        ImGui::InputFloat("Near", &camera->m_NearPlane);
                        ImGui::InputFloat("Far", &camera->m_FarPlane);
                    }
                    Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*camera);
                    ImGui::TreePop();
                }
                if (script && ImGui::TreeNode("ComponentScript")) {
                    auto& scriptData       = m_ComponentScriptContent.at(e.id());
                    ImVec2 ImGUIWindowSize = ImGui::GetWindowContentRegionMax();
                    ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Script");
                    float textboxWidth     = ImGUIWindowSize.x - 120.0f;
                    float textboxHeight    = float(std::max(300, int(ImGUIWindowSize.y)));
                    if (ImGui::InputTextMultiline("##ScriptContent", scriptData.data.data(), 1024, ImVec2(textboxWidth, textboxHeight), ImGuiInputTextFlags_NoHorizontalScroll)) {

                    }
                    if (ImGui::Button("Update", ImVec2(50, 25))) {
                        script->init(scriptData.data.data(), false);
                    }
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

            auto lamda_point_light_data = [](auto& light) {
                auto transform = light.getComponent<ComponentTransform>();
                ImGui::InputDouble3("Position", &transform->m_Position[0]);
                auto constant_ = light.getConstant();
                auto linear_   = light.getLinear();
                auto exponent_ = light.getExponent();
                ImGui::SliderFloat("Constant", &constant_, 0.0f, 5.0f);
                ImGui::SliderFloat("Linear", &linear_, 0.0f, 5.0f);
                ImGui::SliderFloat("Exponent", &exponent_, 0.0f, 5.0f);
                light.setConstant(constant_);
                light.setLinear(linear_);
                light.setExponent(exponent_);
                static const char* AttenuModels[] = { "Constant", "Distance", "Distance Squared", "Constant Linear Exponent", "Distance Radius Squared" };
                static int curr_atten_model       = int(light.getAttenuationModel());
                ImGui::ListBox("Attenuation Model", &curr_atten_model, AttenuModels, IM_ARRAYSIZE(AttenuModels));
                light.setAttenuationModel(static_cast<LightAttenuation>(curr_atten_model));
            };

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
                lamda_point_light_data(light);
            });
            lamda_lights(spotLights, "Spot Lights", "Spot Light", [&](SpotLight& light) {
                lamda_point_light_data(light);
                auto cutoff      = light.getCutoff();
                auto outerCutoff = light.getCutoffOuter();
                ImGui::SliderFloat("Inner Cutoff", &cutoff, 0.0f, 180.0f);
                ImGui::SliderFloat("Outer Cutoff", &outerCutoff, 0.0f, 180.0f);
                light.setCutoffDegrees(cutoff);
                light.setCutoffOuterDegrees(outerCutoff);
            });
            lamda_lights(rodLights, "Rod Lights", "Rod Light", [&](RodLight& light) {
                lamda_point_light_data(light);
                auto rodLength = light.getRodLength();
                ImGui::SliderFloat("Rod Length", &rodLength, 0.0f, 50.0f);
                light.setRodLength(rodLength);
            });
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
}
void Engine::priv::EditorWindowScene::internal_render_profiler(Scene& currentScene) {
    auto& debugging = Engine::priv::Core::m_Engine->m_DebugManager;
    const ImVec4 yellow   = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::TextColored(yellow, std::string("Update Time:  " + debugging.getTimeInMs(DebugTimerTypes::Logic) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Physics Time: " + debugging.getTimeInMs(DebugTimerTypes::Physics) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Sounds Time:  " + debugging.getTimeInMs(DebugTimerTypes::Sound) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Render Time:  " + debugging.getTimeInMs(DebugTimerTypes::Render) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Delta Time:   " + debugging.deltaTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, std::string("FPS:  " + debugging.fps()).c_str());

#ifdef _WIN32

    m_Strm.str({});
    m_Strm.clear();
    m_Strm << std::fixed << std::setprecision(4);

    auto byte_format = [](DWORDLONG input, std::stringstream& strm) {
        strm.str({});
        strm.clear();
        if (input >= 1099511627776) {
            strm << (double(input) / 1099511627776.0) << " TB";
        } else if (input >= 1073741824) {
            strm << (double(input) / 1073741824.0) << " GB";
        } else if (input >= 1048576) {
            strm << (double(input) / 1048576.0) << " MB";
        } else if (input >= 1024) {
            strm << (double(input) / 1024.0) << " KB";
        } else {
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

    ImGui::TextColored(yellow, std::string("Total Virtual Memory: " + byte_format(totalVirtualMem, m_Strm)).c_str());
    ImGui::TextColored(yellow, std::string("Virtual Memory Used: " + byte_format(virtualMemUsed, m_Strm)).c_str());
    ImGui::TextColored(yellow, std::string("Total Physical Memory (RAM): " + byte_format(totalPhysMem, m_Strm)).c_str());
    ImGui::TextColored(yellow, std::string("Physical Memory Used: " + byte_format(physMemUsed, m_Strm)).c_str());

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage; //Virtual Memory currently used by current process
    SIZE_T physMemUsedByMe    = pmc.WorkingSetSize; //Physical Memory currently used by current process
    ImGui::TextColored(yellow, std::string("Virtual Memory used by current process: " + byte_format(virtualMemUsedByMe, m_Strm)).c_str());
    ImGui::TextColored(yellow, std::string("Physical Memory used by current process: " + byte_format(physMemUsedByMe, m_Strm)).c_str());
#endif

}
void Engine::priv::EditorWindowScene::internal_render_renderer(Scene& currentScene) {
    auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
    //general
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "General");
        bool vsync = Engine::Resources::getWindow().isVsyncEnabled();
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
        ImGui::SliderFloat("Bloom Strength", &Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Strength, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Exposure", &Engine::priv::Bloom::STATIC_BLOOM.m_Exposure, -5.0f, 5.0f);
        ImGui::SliderFloat("Bloom Threshold", &Engine::priv::Bloom::STATIC_BLOOM.m_Threshold, -5.0f, 5.0f);
        ImGui::SliderFloat("Bloom Scale", &Engine::priv::Bloom::STATIC_BLOOM.m_Scale, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Blur Radius", &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Radius, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Blur Strength", &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Strength, -5.0f, 5.0f);
        static int bloom_samples = Engine::priv::Bloom::STATIC_BLOOM.m_Num_Passes;
        ImGui::SliderInt("Bloom Num Passes", &bloom_samples, 0, 16);
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
void Engine::priv::EditorWindowScene::internal_render_resources(Scene& currentScene) {
    ImGui::BeginChild("ChildResources");
    if (ImGui::TreeNode("Materials")) {
        auto materials = Engine::Resources::GetAllResourcesOfType<Material>(); //doing this every frame is slow
        for (auto& material : materials) {
            if (ImGui::TreeNode(material->name().c_str())) {

                const auto& color = material->getF0();
                float color_arr[] = { color.r(), color.g(), color.b(), color.a() };
                ImGui::ColorEdit3("F0 Color", &color_arr[0]);
                material->setF0Color(uint8_t(color_arr[0] * 255.0f), uint8_t(color_arr[1] * 255.0f), uint8_t(color_arr[2] * 255.0f));

                int glow = int(material->getGlow());
                ImGui::SliderInt("Glow", &glow, 0, 255);
                material->setGlow(uint8_t(glow));

                int alpha = int(material->getAlpha());
                ImGui::SliderInt("Alpha", &alpha, 0, 255);
                material->setAlpha(uint8_t(alpha));

                int ao = int(material->getAO());
                ImGui::SliderInt("AO", &ao, 0, 255);
                material->setAO(uint8_t(ao));

                int metalness = int(material->getMetalness());
                ImGui::SliderInt("Metalness", &metalness, 0, 255);
                material->setMetalness(uint8_t(metalness));

                int smoothness = int(material->getSmoothness());
                ImGui::SliderInt("Smoothness", &smoothness, 0, 255);
                material->setSmoothness(uint8_t(smoothness));

                bool isShadeless = material->getShadeless();
                ImGui::Checkbox("Shadeless", &isShadeless);
                material->setShadeless(isShadeless);

                static const char* DiffuseModels[] = { "None", "Lambert", "Oren Nayar", "Ashikhmin Shirley", "Minnaert" };
                int diffuseModel = int(material->getDiffuseModel());
                ImGui::ListBox("Diffuse Model", &diffuseModel, DiffuseModels, IM_ARRAYSIZE(DiffuseModels));
                material->setDiffuseModel(static_cast<DiffuseModel>(diffuseModel));

                static const char* SpecularModels[] = { "None", "Blinn Phong", "Phong", "GGX", "Cook Torrance", "Guassian", "Beckmann", "Ashikhmin Shirley"};
                int specularModel = int(material->getSpecularModel());
                ImGui::ListBox("Specular Model", &specularModel, SpecularModels, IM_ARRAYSIZE(SpecularModels));
                material->setSpecularModel(static_cast<SpecularModel>(specularModel));

                ImGui::TreePop();
                ImGui::Separator();
            }
        }
        ImGui::TreePop();
        ImGui::Separator();
    }
    ImGui::EndChild();
}
void Engine::priv::EditorWindowScene::update() {
    auto currScene        = Engine::Resources::getCurrentScene();
    std::string sceneName = currScene ? currScene->name() : "N/A";
    ImGui::Begin(("Scene - " + sceneName).c_str(), NULL);
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    if (ImGui::BeginTabBar("TabBar")) {
        for (int i = 0; i < EditorWindowSceneFunctions::TAB_TYPES_DATA.size(); ++i) {
            if (ImGui::BeginTabItem(std::get<0>(EditorWindowSceneFunctions::TAB_TYPES_DATA[i]))) {
                m_Tab = i;
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleColor();
    (this->*std::get<1>(EditorWindowSceneFunctions::TAB_TYPES_DATA[m_Tab]))(*currScene);

    // Plot some values
    //const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
    //ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

    ImGui::End();
}