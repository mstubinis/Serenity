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
#include <serenity/resources/shader/Shader.h>

#include <serenity/networking/Networking.h>

#include <iomanip>
#include <fstream>

#include <serenity/resources/mesh/smsh.h>

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#endif

struct EditorWindowSceneFunction {
    using FunctionType = void(*)(Scene&);

    const char*   title;
    FunctionType  function;
};
struct ScriptContent {
    std::string  data;
    bool         fromFile = false;
};
namespace {
    std::stringstream                               STR_STREAM;
    std::unordered_map<uint32_t, ScriptContent>     COMPONENT_SCRIPT_CONTENT; //entity id => ScriptContent(string, bool)
    std::unordered_map<std::string, ScriptContent>  SHADER_CONTENT; //shader name => ScriptContent(string, bool)
}
namespace Engine::priv {
    class EditorWindowSceneFunctions {
        public: 
            static void internal_render_entities(Scene& currentScene);
            static void internal_render_systems(Scene& currentScene);
            static void internal_render_renderer(Scene& currentScene);
            static void internal_render_resources(Scene& currentScene);
            static void internal_render_profiler(Scene& currentScene);
            static void internal_render_network(Scene& currentScene);

            static void internal_render_entity(Entity);

            constexpr static std::array<EditorWindowSceneFunction, Engine::priv::EditorWindowScene::TabType::_TOTAL> TAB_TYPES_DATA { {
                { "Entities", &internal_render_entities },
                { "Systems", &internal_render_systems },
                { "Renderer", &internal_render_renderer },
                { "Resources", &internal_render_resources },
                { "Profiler", &internal_render_profiler },
                { "Network", &internal_render_network },
            } };
    };
}

void Engine::priv::EditorWindowSceneFunctions::internal_render_entity(Entity e) {
    auto name = e.getComponent<ComponentName>();
    if (ImGui::TreeNode(("Entity " + std::to_string(e.id()) + (name ? (" - " + name->name()) : "")).c_str())) {
        //for each component...
        auto transform = e.getComponent<ComponentTransform>();
        auto rigid = e.getComponent<ComponentRigidBody>();
        auto shape = e.getComponent<ComponentCollisionShape>();
        auto model = e.getComponent<ComponentModel>();
        auto camera = e.getComponent<ComponentCamera>();
        auto script = e.getComponent<ComponentScript>();
        if (name && ImGui::TreeNode("ComponentName")) {
            ImGui::Text(std::string("Name: " + name->name()).c_str());
            ImGui::TreePop();
        }
        if (transform && ImGui::TreeNode("ComponentTransform")) {
            ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Position"); ImGui::SameLine(); ImGui::InputDouble3("##trans_pos", &transform->m_Position[0]);
            ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Rotation"); ImGui::SameLine(); ImGui::InputFloat4("##trans_rot", &transform->m_Rotation[0]);
            ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Scale   "); ImGui::SameLine(); ImGui::InputFloat3("##trans_scl", &transform->m_Scale[0]);

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
            ImGui::Text(std::string("Radius: " + std::to_string(model->getRadius())).c_str());
            const std::string bbX = std::to_string(model->getBoundingBox().x);
            const std::string bbY = std::to_string(model->getBoundingBox().y);
            const std::string bbZ = std::to_string(model->getBoundingBox().z);
            ImGui::Text(std::string("BoundingBox: " + bbX + ", " + bbY + ", " + bbZ).c_str());
            for (size_t i = 0; i < model->getNumModels(); ++i) {
                ModelInstance& instance = model->getModel(i);
                if (ImGui::TreeNode(("ModelInstance " + std::to_string(i)).c_str())) {
                    const auto& color = instance.getColor();
                    const auto& GRColor = instance.getGodRaysColor();
                    float aColor[4] = { color.r(), color.g(), color.b(), color.a() };
                    float aGodRays[3] = { GRColor.r(), GRColor.g(), GRColor.b() };
                    if (ImGui::ColorEdit4("Color", &aColor[0])) {
                        instance.setColor(aColor[0], aColor[1], aColor[2], aColor[3]);
                    }
                    if (ImGui::ColorEdit3("God Rays Color", &aGodRays[0])) {
                        instance.setGodRaysColor(aGodRays[0], aGodRays[1], aGodRays[2]);
                    }

                    ImGui::Checkbox("Force Render", &instance.m_ForceRender);
                    ImGui::SameLine();
                    ImGui::Checkbox("Cast Shadow", &instance.m_IsShadowCaster);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show", &instance.m_Visible);
                    ImGui::SameLine();

                    ImGui::Separator();

                    auto pos = instance.getPosition();
                    if (ImGui::InputFloat3("position", &pos[0])) {
                        instance.setPosition(pos);
                    }

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

                    instance.internal_update_model_matrix(true);

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (camera && ImGui::TreeNode("ComponentCamera")) {
            if (camera->getType() == ComponentCamera::CameraType::Perspective) {
                ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Perspective Camera");
                ImGui::SliderFloat("Angle", &camera->m_AngleOrLeft, 0.0f, glm::radians(180.0f));
                ImGui::InputFloat("AspectRatio", &camera->m_AspectRatioOrRight);
                ImGui::InputFloat("Near", &camera->m_NearPlane);
                ImGui::InputFloat("Far", &camera->m_FarPlane);
            }
            else {
                ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Orthographic Camera");
                ImGui::InputFloat("Left", &camera->m_AngleOrLeft);
                ImGui::InputFloat("Right", &camera->m_AspectRatioOrRight);
                ImGui::InputFloat("Top", &camera->m_Top);
                ImGui::InputFloat("Bottom", &camera->m_Bottom);
                ImGui::InputFloat("Near", &camera->m_NearPlane);
                ImGui::InputFloat("Far", &camera->m_FarPlane);
            }
            Engine::priv::ComponentCamera_Functions::RebuildProjectionMatrix(*camera);
            ImGui::TreePop();
        }
        if (script && ImGui::TreeNode("ComponentScript")) {
            auto& scriptData = COMPONENT_SCRIPT_CONTENT.at(e.id());
            ImVec2 ImGUIWindowSize = ImGui::GetWindowContentRegionMax();
            ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Script");
            float textboxWidth = ImGUIWindowSize.x - 120.0f;
            float textboxHeight = float(std::max(300, int(ImGUIWindowSize.y)));
            if (ImGui::InputTextMultiline("##ScriptContent", scriptData.data.data(), scriptData.data.size() + 1024, ImVec2(textboxWidth, textboxHeight), ImGuiInputTextFlags_NoHorizontalScroll)) {

            }
            if (ImGui::Button("Update", ImVec2(50, 25))) {
                script->init(scriptData.data.data());
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}
void Engine::priv::EditorWindowSceneFunctions::internal_render_network(Scene& currentScene) {
    const ImVec4 yellow      = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const auto& tcpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.getSocketManager().getTCPSockets();
    const auto& tcpListeners = Engine::priv::Core::m_Engine->m_NetworkingModule.getSocketManager().getTCPListeners();
    const auto& udpSockets   = Engine::priv::Core::m_Engine->m_NetworkingModule.getSocketManager().getUDPSockets();
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
void Engine::priv::EditorWindowSceneFunctions::internal_render_entities(Scene& currentScene) {
    ImGui::BeginChild("SceneEntities");
    const auto& entities = Engine::priv::PublicScene::GetEntities(currentScene);
    if (ImGui::TreeNode("Entities")) {
        for (const Entity e : entities) {
            internal_render_entity(e);
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
        if (ImGui::Checkbox("Enabled", &isLightActive)) {
            light.activate(isLightActive);
        }

        bool isShadowCaster = light.isShadowCaster();
        if (ImGui::Checkbox("Casts Shadows", &isShadowCaster)) {
            light.setShadowCaster(isShadowCaster); //TODO: currently, the default args for this override whatever was used previously.
        }

        const auto& color = light.getColor();
        if (ImGui::ColorEdit4("Color", const_cast<float*>(glm::value_ptr(color)))) {
            light.setColor(color);
        }

        auto diff = light.getDiffuseIntensity();
        auto spec = light.getSpecularIntensity();
        if (ImGui::SliderFloat("Diffuse Intensity", &diff, 0.0f, 20.0f)) {
            light.setDiffuseIntensity(diff);
        }
        if (ImGui::SliderFloat("Specular Intensity", &spec, 0.0f, 20.0f)) {


        }
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
                if (ImGui::SliderFloat("Constant", &constant_, 0.0f, 5.0f)) {
                    light.setConstant(constant_);
                }
                if (ImGui::SliderFloat("Linear", &linear_, 0.0f, 5.0f)) {
                    light.setLinear(linear_);
                }
                if (ImGui::SliderFloat("Exponent", &exponent_, 0.0f, 5.0f)) {
                    light.setExponent(exponent_);
                }
                static const char* AttenuModels[] = { "Constant", "Distance", "Distance Squared", "Constant Linear Exponent", "Distance Radius Squared" };
                static int curr_atten_model       = int(light.getAttenuationModel());
                if (ImGui::ListBox("Attenuation Model", &curr_atten_model, AttenuModels, IM_ARRAYSIZE(AttenuModels))) {
                    light.setAttenuationModel(static_cast<LightAttenuation>(curr_atten_model));
                }
            };

            lamda_lights(directionalLights, "Directional Lights", "Directional Light", [&](DirectionalLight& light) {
                const auto dir = light.getDirection();
                if (ImGui::InputFloat3("Direction", const_cast<float*>(glm::value_ptr(dir)))) {
                    light.setDirection(dir); //TODO: add a check to see if the direction is different in this function call? this is quite expensive to do per frame
                }
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
                if (ImGui::SliderFloat("Inner Cutoff", &cutoff, 0.0f, 180.0f)) {
                    light.setCutoffDegrees(cutoff);
                }
                if (ImGui::SliderFloat("Outer Cutoff", &outerCutoff, 0.0f, 180.0f)) {
                    light.setCutoffOuterDegrees(outerCutoff);
                }
            });
            lamda_lights(rodLights, "Rod Lights", "Rod Light", [&](RodLight& light) {
                lamda_point_light_data(light);
                auto rodLength = light.getRodLength();
                if (ImGui::SliderFloat("Rod Length", &rodLength, 0.0f, 50.0f)) {
                    light.setRodLength(rodLength);
                }
            });
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    ImGui::EndChild();
}
void Engine::priv::EditorWindowSceneFunctions::internal_render_systems(Scene& currentScene) {
#ifndef ENGINE_PRODUCTION
    ImGui::BeginChild("SceneSystems");
    auto& systems = Engine::priv::PublicScene::GetECS(currentScene).getSystemPool();
    if (ImGui::TreeNode("Systems")) {
        systems.forEachOrdered([](SystemBaseClass* system) {
            const char* systemName = typeid(*system).name();
            if (ImGui::TreeNode(systemName)) {
                const auto& entities = system->getEntities();
                const auto& associatedComponents = system->getAssociatedComponents();
                if (ImGui::TreeNode(("Entities (" + std::to_string(entities.size()) + ")").c_str())) {
                    for (const Entity e : entities) {
                        internal_render_entity(e);
                    }
                    ImGui::TreePop();
                    ImGui::Separator();
                }
                if (associatedComponents.size() > 0) {
                    if (ImGui::TreeNode(("Component Pools (" + std::to_string(associatedComponents.size()) + ")").c_str())) {
                        for (const auto& associatedComponentData : associatedComponents) {
                            const auto ComponentClassName = associatedComponentData.pool->getComponentDebugName();
                            if (ImGui::TreeNode(ComponentClassName)) {
                                ImGui::TreePop();
                                ImGui::Separator();
                            }
                        }
                        ImGui::TreePop();
                        ImGui::Separator();
                    }
                }
                ImGui::TreePop();
                ImGui::Separator();
            }
        });
        ImGui::TreePop();
        ImGui::Separator();
    }
    ImGui::EndChild();
#endif
}
void Engine::priv::EditorWindowSceneFunctions::internal_render_profiler(Scene& currentScene) {
    auto& debugging = Engine::priv::Core::m_Engine->m_DebugManager;
    const ImVec4 yellow   = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::TextColored(yellow, std::string("Update Time:  " + debugging.getTimeInMs(DebugTimerTypes::Logic) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Physics Time: " + debugging.getTimeInMs(DebugTimerTypes::Physics) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Sounds Time:  " + debugging.getTimeInMs(DebugTimerTypes::Sound) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Render Time:  " + debugging.getTimeInMs(DebugTimerTypes::Render) + " ms").c_str());
    ImGui::TextColored(yellow, std::string("Delta Time:   " + debugging.deltaTimeInMs() + " ms").c_str());
    ImGui::TextColored(yellow, std::string("FPS:  " + debugging.fps()).c_str());

#ifdef _WIN32

    STR_STREAM.str({});
    STR_STREAM.clear();
    STR_STREAM << std::fixed << std::setprecision(4);

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

    ImGui::TextColored(yellow, std::string("Total Virtual Memory: " + byte_format(totalVirtualMem, STR_STREAM)).c_str());
    ImGui::TextColored(yellow, std::string("Virtual Memory Used: " + byte_format(virtualMemUsed, STR_STREAM)).c_str());
    ImGui::TextColored(yellow, std::string("Total Physical Memory (RAM): " + byte_format(totalPhysMem, STR_STREAM)).c_str());
    ImGui::TextColored(yellow, std::string("Physical Memory Used: " + byte_format(physMemUsed, STR_STREAM)).c_str());

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage; //Virtual Memory currently used by current process
    SIZE_T physMemUsedByMe    = pmc.WorkingSetSize; //Physical Memory currently used by current process
    ImGui::TextColored(yellow, std::string("Virtual Memory used by current process: " + byte_format(virtualMemUsedByMe, STR_STREAM)).c_str());
    ImGui::TextColored(yellow, std::string("Physical Memory used by current process: " + byte_format(physMemUsedByMe, STR_STREAM)).c_str());
#endif

}
void Engine::priv::EditorWindowSceneFunctions::internal_render_renderer(Scene& currentScene) {
    auto& renderer = Engine::priv::Core::m_Engine->m_RenderModule;
    //general
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "General");
        bool vsync = Engine::Resources::getWindow().isVsyncEnabled();
        if (ImGui::Checkbox("Vsync ", &vsync)) {
            Engine::Resources::getWindow().setVerticalSyncEnabled(vsync);
        }
    }
    //skybox
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Skybox");
        ImGui::Checkbox("Skybox Enabled ", &renderer.m_DrawSkybox);
    }
    //lighting
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Lighting");
        constexpr std::array<const char*, 2> LightingModels = { "Basic", "Physical" };
        ImGui::Checkbox("Lighting Enabled", &renderer.m_Lighting);
        ImGui::SliderFloat("GI Contribution Diffuse", &renderer.m_GI_Diffuse, 0.0f, 1.0f);
        ImGui::SliderFloat("GI Contribution Specular", &renderer.m_GI_Specular, 0.0f, 1.0f);
        ImGui::SliderFloat("GI Contribution Global", &renderer.m_GI_Global, 0.0f, 1.0f);
        renderer.m_GI_Pack = Engine::Compression::pack3FloatsInto1FloatUnsigned(renderer.m_GI_Diffuse, renderer.m_GI_Specular, renderer.m_GI_Global);
        static int lighting_model_current   = int(renderer.m_LightingAlgorithm);
        if (ImGui::ListBox("Lighting Model", &lighting_model_current, LightingModels.data(), int(LightingModels.size()))) {
            Engine::Renderer::Settings::Lighting::setLightingAlgorithm(lighting_model_current);
        }
        if (lighting_model_current == LightingAlgorithm::Basic) {
            auto scene = Engine::Resources::getCurrentScene();
            if (scene) {
                glm::vec3 ambientColor = scene->getAmbientColor();
                if (ImGui::ColorEdit3("Ambient Color", &ambientColor[0])) {
                    scene->setAmbientColor(ambientColor[0], ambientColor[1], ambientColor[2]);
                }
            }
        }
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Shadow Settings");
        ImGui::SliderFloat("Z Mult Factor", &renderer.m_ShadowZMultFactor, 0.0f, 10.0f);
        ImGui::SliderFloat("Clipspace offset", &renderer.m_ShadowClipspaceOffset, 0.0f, 3.0f);

        ImGui::Separator();
    }
    //hdr
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "HDR");
        constexpr std::array<const char*, 5> HDRAlgos = { "None", "Reinhard", "Filmic", "Exposure", "Uncharted" };
        static int hdr_algo_current   = int(Engine::priv::HDR::STATIC_HDR.m_Algorithm);
        if (ImGui::ListBox("HDR Algorithm", &hdr_algo_current, HDRAlgos.data(), int(HDRAlgos.size()))) {
            Engine::Renderer::hdr::setAlgorithm(hdr_algo_current);
        }
        ImGui::SliderFloat("HDR Exposure", &Engine::priv::HDR::STATIC_HDR.m_Exposure, -15.0f, 15.0f);
        ImGui::Separator();
    }
    //bloom
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Bloom");
        ImGui::Checkbox("Bloom Enabled",          &Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Active);
        ImGui::SliderFloat("Bloom Strength",      &Engine::priv::Bloom::STATIC_BLOOM.m_Bloom_Strength,  0.0f,  5.0f);
        ImGui::SliderFloat("Bloom Exposure",      &Engine::priv::Bloom::STATIC_BLOOM.m_Exposure,       -5.0f,  5.0f);
        ImGui::SliderFloat("Bloom Threshold",     &Engine::priv::Bloom::STATIC_BLOOM.m_Threshold,      -5.0f,  5.0f);
        ImGui::SliderFloat("Bloom Scale",         &Engine::priv::Bloom::STATIC_BLOOM.m_Scale,           0.0f,  5.0f);
        ImGui::SliderFloat("Bloom Blur Radius",   &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Radius,     0.0f,  5.0f);
        ImGui::SliderFloat("Bloom Blur Strength", &Engine::priv::Bloom::STATIC_BLOOM.m_Blur_Strength,  -5.0f,  5.0f);
        ImGui::SliderInt("Bloom Num Passes",      &Engine::priv::Bloom::STATIC_BLOOM.m_Num_Passes,      0,     16);
        ImGui::Separator();
    }
    //ssao
    {
        constexpr std::array<const char*, 5> SSAOLevels = { "Off", "Low", "Medium", "High", "Ultra" };
        static int ssao_algo_current    = Engine::priv::SSAO::STATIC_SSAO.m_SSAOLevel;
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Screen Space Ambient Occlusion");
        if (ImGui::ListBox("SSAO Level", &ssao_algo_current, SSAOLevels.data(), int(SSAOLevels.size()))) {
            Engine::Renderer::ssao::setLevel(ssao_algo_current);
        }
        ImGui::SliderFloat("SSAO Bias",          &Engine::priv::SSAO::STATIC_SSAO.m_Bias,           -1.0f,  1.0f);
        ImGui::SliderFloat("SSAO Scale",         &Engine::priv::SSAO::STATIC_SSAO.m_Scale,           0.0f,  3.0f);
        ImGui::SliderFloat("SSAO Radius",        &Engine::priv::SSAO::STATIC_SSAO.m_Radius,          0.0f,  5.0f);
        ImGui::SliderFloat("SSAO Intensity",     &Engine::priv::SSAO::STATIC_SSAO.m_Intensity,       0.0f,  10.0f);
        ImGui::SliderFloat("SSAO Range Scale",   &Engine::priv::SSAO::STATIC_SSAO.m_RangeCheckScale, 0.0f,  3.0f);
        ImGui::SliderInt("SSAO Samples",         &Engine::priv::SSAO::STATIC_SSAO.m_NumSamples,      0,     SSAO_MAX_KERNEL_SIZE);
        ImGui::SliderFloat("SSAO Blur Strength", &Engine::priv::SSAO::STATIC_SSAO.m_BlurStrength,    0.0f,  10.0f);
        ImGui::SliderInt("SSAO Blur Passes",     &Engine::priv::SSAO::STATIC_SSAO.m_BlurNumPasses,   0,     4);
        ImGui::Separator();
    }
    //anti-aliasing
    {
        constexpr std::array<const char*, 6> AAAlgos = { "Off", "FXAA", "SMAA Low", "SMAA Medium", "SMAA High", "SMAA Ultra" };
        static int aa_algo_current   = int(renderer.m_AA_algorithm);
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Anti-Aliasing");
        if (ImGui::ListBox("Anti-Aliasing Algorithm", &aa_algo_current, AAAlgos.data(), int(AAAlgos.size()))) {
            Engine::Renderer::Settings::setAntiAliasingAlgorithm(aa_algo_current);
        }
        ImGui::Separator();
    }
    //debugging
    {
        ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "Debugging");
        ImGui::Checkbox("Draw Physics", &renderer.m_DrawPhysicsDebug);
        ImGui::Separator();
    }
}
void Engine::priv::EditorWindowSceneFunctions::internal_render_resources(Scene& currentScene) {
    ImGui::BeginChild("ChildResources");
    if (ImGui::TreeNode("Materials")) {
        auto materials = Engine::Resources::GetAllResourcesOfType<Material>(); //doing this every frame is slow
        for (auto& material : materials) {
            if (ImGui::TreeNode(material->name().c_str())) {

                const auto& color = material->getF0();
                float color_arr[] = { color.r(), color.g(), color.b(), color.a() };
                if (ImGui::ColorEdit3("F0 Color", &color_arr[0])) {
                    material->setF0Color(uint8_t(color_arr[0] * 255.0f), uint8_t(color_arr[1] * 255.0f), uint8_t(color_arr[2] * 255.0f));
                }

                int glow = int(material->getGlow());
                if (ImGui::SliderInt("Glow", &glow, 0, 255)) {
                    material->setGlow(uint8_t(glow));
                }

                int alpha = int(material->getAlpha());
                if (ImGui::SliderInt("Alpha", &alpha, 0, 255)) {
                    material->setAlpha(uint8_t(alpha));
                }

                int ao = int(material->getAO());
                if (ImGui::SliderInt("AO", &ao, 0, 255)) {
                    material->setAO(uint8_t(ao));
                }

                int metalness = int(material->getMetalness());
                if (ImGui::SliderInt("Metalness", &metalness, 0, 255)) {
                    material->setMetalness(uint8_t(metalness));
                }

                int smoothness = int(material->getSmoothness());
                if (ImGui::SliderInt("Smoothness", &smoothness, 0, 255)) {
                    material->setSmoothness(uint8_t(smoothness));
                }

                bool isShadeless = material->getShadeless();
                if (ImGui::Checkbox("Shadeless", &isShadeless)) {
                    material->setShadeless(isShadeless);
                }

                static const char* DiffuseModels[] = { "None", "Lambert", "Oren Nayar", "Ashikhmin Shirley", "Minnaert" };
                int diffuseModel = material->getDiffuseModel();
                if (ImGui::ListBox("Diffuse Model", &diffuseModel, DiffuseModels, IM_ARRAYSIZE(DiffuseModels))) {
                    material->setDiffuseModel(diffuseModel);
                }

                static const char* SpecularModels[] = { "None", "Blinn Phong", "Phong", "GGX", "Cook Torrance", "Guassian", "Beckmann", "Ashikhmin Shirley"};
                int specularModel = material->getSpecularModel();
                if (ImGui::ListBox("Specular Model", &specularModel, SpecularModels, IM_ARRAYSIZE(SpecularModels))) {
                    material->setSpecularModel(specularModel);
                }

                ImGui::TreePop();
                ImGui::Separator();
            }
        }
        ImGui::TreePop();
        ImGui::Separator();
    }
    if (ImGui::TreeNode("Meshes")) {
        auto meshes = Engine::Resources::GetAllResourcesOfType<Mesh>(); //doing this every frame is slow
        for (auto& mesh : meshes) {
            if (ImGui::TreeNode(mesh->name().c_str())) {



                ImGui::TreePop();
                ImGui::Separator();
            }
        }
        ImGui::TreePop();
        ImGui::Separator();
    }
    if (ImGui::TreeNode("Shader Programs")) {
        auto shaderPrograms = Engine::Resources::GetAllResourcesOfType<ShaderProgram>(); //doing this every frame is slow
        for (auto& shaderProgram : shaderPrograms) {
            if (ImGui::TreeNode(shaderProgram->name().c_str())) {

                for (const auto& [shaderHandle, shaderType] : shaderProgram->getShaders()) {
                    Shader* shader = shaderHandle.get<Shader>();
                    if (ImGui::TreeNode(shader->name().c_str())) {
                        auto& scriptData = SHADER_CONTENT.at(shader->name());
                        ImVec2 ImGUIWindowSize = ImGui::GetWindowContentRegionMax();
                        ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "Shader Code");
                        float textboxWidth = ImGUIWindowSize.x - 120.0f;
                        float textboxHeight = float(std::max(300, int(ImGUIWindowSize.y)));
                        if (ImGui::InputTextMultiline("##ShaderContent", scriptData.data.data(), scriptData.data.size() + 1024, ImVec2(textboxWidth, textboxHeight), ImGuiInputTextFlags_NoHorizontalScroll)) {

                        }
                        if (ImGui::Button("Update", ImVec2(50, 25))) {
                            shader->load(scriptData.data.data());
                            shaderProgram->unload();
                            shaderProgram->load();
                        }
                        ImGui::TreePop();
                    }
                }
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
            if (ImGui::BeginTabItem(EditorWindowSceneFunctions::TAB_TYPES_DATA[i].title)) {
                m_Tab = i;
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleColor();
    EditorWindowSceneFunctions::TAB_TYPES_DATA[m_Tab].function(*currScene);

    ImGui::End();
}
bool Engine::priv::EditorWindowScene::addComponentScriptData(uint32_t entityID, std::string_view scriptFilePathOrData, bool isFile) {
    if (isFile) {
        std::ifstream fileStream = std::ifstream(std::string{ scriptFilePathOrData });
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        COMPONENT_SCRIPT_CONTENT[entityID].data = buffer.str();
        COMPONENT_SCRIPT_CONTENT[entityID].fromFile = isFile;
    } else {
        COMPONENT_SCRIPT_CONTENT[entityID].data = std::string{ scriptFilePathOrData };
        COMPONENT_SCRIPT_CONTENT[entityID].fromFile = isFile;
    }
    return true;
}
void Engine::priv::EditorWindowScene::addShaderData(Shader& shader, std::string_view shaderCode) {
    SHADER_CONTENT[shader.name()].data     = shader.getData();
    SHADER_CONTENT[shader.name()].fromFile = shader.isFromFile();
}