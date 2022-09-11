#include <serenity/editor/core/EditorCore.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/editor/imgui/imgui.h>
#include <serenity/editor/imgui/imgui-SFML.h>
#include <serenity/editor/imgui/imgui_impl_opengl3.h>
#include <serenity/system/window/Window.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/lights/Lights.h>

#include <serenity/editor/windowScene/WindowScene.h>
#include <serenity/editor/embeddedImages/PointLightImage.h>
#include <serenity/editor/embeddedImages/SunLightImage.h>
#include <serenity/editor/embeddedImages/SpotLightImage.h>
#include <serenity/editor/embeddedImages/RodLightImage.h>

#include <serenity/ecs/components/ComponentScript.h>



Engine::view_ptr<Engine::priv::EditorCore> Engine::priv::EditorCore::EDITOR;

namespace {
    Handle internal_load_embedded_image(const uint8_t* data, int width, int height, const char* textureName) {
        std::vector<uint8_t> pixels(width * height * 4);
        auto setPixel = [&pixels, width](uint32_t y, uint32_t x, const std::array<uint8_t, 4>& color) {
            pixels[(y * width + x) * 4 + 0] = color[0];
            pixels[(y * width + x) * 4 + 1] = color[1];
            pixels[(y * width + x) * 4 + 2] = color[2];
            pixels[(y * width + x) * 4 + 3] = color[3];
        };
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const auto pixel = data[(y * width) + x];
                if (pixel == 128) {
                    setPixel(height - y - 1, x, { 0, 0, 0, 0 });
                } else if (pixel == 0) {
                    setPixel(height - y - 1, x, { 0, 0, 0, 255 });
                } else if (pixel == 255) {
                    setPixel(height - y - 1, x, { 255, 255, 255, 255 });
                } else {
                    setPixel(height - y - 1, x, { 128, 128, 128, 255 });
                }
            }
        }
        Handle textureHandle = Engine::Resources::loadTexture(pixels.data(), width, height, textureName, ImageInternalFormat::SRGB8_ALPHA8, false, false);
        textureHandle.get<Texture>()->setFilter(TextureFilter::Nearest);
        return textureHandle;
    }
}

Engine::priv::EditorCore::EditorCore(const EngineOptions& options, Window& window) {
    m_Enabled = options.editor_enabled;
    if (m_Enabled) {
        m_WindowScene = NEW Engine::priv::EditorWindowScene{};
        ImGui::SFML::Init(window.getSFMLHandle(), static_cast<sf::Vector2f>(window.getSFMLHandle().getSize()));
        ImGui_ImplOpenGL3_Init();
        m_RegisteredWindows.insert(&window);

        m_PointLightTexture = internal_load_embedded_image(POINT_LIGHT_IMAGE_DATA, 32, 32, "PointLightEditorTexture");
        m_SunLightTexture   = internal_load_embedded_image(SUN_LIGHT_IMAGE_DATA, 32, 32, "SunLightEditorTexture");
        m_SpotLightTexture  = internal_load_embedded_image(SPOT_LIGHT_IMAGE_DATA, 32, 32, "SpotLightEditorTexture");
        m_RodLightTexture   = internal_load_embedded_image(ROD_LIGHT_IMAGE_DATA, 32, 32, "RodLightEditorTexture");
    }
    EDITOR = this;
}
Engine::priv::EditorCore::~EditorCore() {
    SAFE_DELETE(m_WindowScene);
    if (m_Enabled) {
        ImGui::SFML::Shutdown();
        m_RegisteredWindows.clear();
    }
}
bool Engine::priv::EditorCore::addComponentScriptData(Entity entity, std::string_view scriptFilePathOrData, bool isFile) {
    return m_WindowScene->addComponentScriptData(entity.id(), scriptFilePathOrData, isFile);
}
void Engine::priv::EditorCore::addShaderData(Shader& shader, std::string_view shaderCode) {
    m_WindowScene->addShaderData(shader, shaderCode);
}

bool Engine::priv::EditorCore::isWindowRegistered(Window& window) const noexcept {
    return m_RegisteredWindows.contains(&window);
}
void Engine::priv::EditorCore::processEvent(const sf::Event& e) {
    if (m_Enabled && m_Shown) {
        ImGui::SFML::ProcessEvent(e);
    }
}
void Engine::priv::EditorCore::update(Window& window, const float dt) {
    if (m_Enabled && m_Shown && isWindowRegistered(window)) {
        ImGui::SFML::Update(window.getSFMLHandle(), sf::seconds(dt));
        ImGui::NewFrame();
        m_WindowScene->update();
        ImGui::EndFrame();
    }
}
void Engine::priv::EditorCore::render(Window& window) {
    if (m_Enabled && m_Shown && isWindowRegistered(window)) {
        //ImGui::SFML::Render(window.getSFMLHandle());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
void Engine::priv::EditorCore::renderLightIcons(Scene& scene, Viewport& viewport) {
    if (m_Enabled && m_Shown && (viewport.getRenderFlags() & ViewportRenderingFlag::API2D)) {
        //auto& directionalLights = Engine::priv::PublicScene::GetLights<DirectionalLight>(scene);
        auto& sunLights         = Engine::priv::PublicScene::GetLights<SunLight>(scene);
        auto& pointLights       = Engine::priv::PublicScene::GetLights<PointLight>(scene);
        auto& spotLights        = Engine::priv::PublicScene::GetLights<SpotLight>(scene);
        auto& rodLights         = Engine::priv::PublicScene::GetLights<RodLight>(scene);
        if (scene.getActiveCamera()) {
            auto render_light_icons = [&scene, &viewport](auto& container, Handle texture) {
                const auto depth = 0.1f;
                for (const auto& light : container) {
                    const auto twoDPos = Engine::Math::getScreenCoordinates(
                        light->getComponent<ComponentTransform>()->getWorldPosition(),
                        *scene.getActiveCamera(), 
                        viewport.getViewportDimensions(),
                        false
                    );
                    if (twoDPos.z > 0) {
                        Engine::Renderer::renderTexture(texture, glm::vec2{ twoDPos }, light->getColor(), 0.0f, glm::vec2{ 1.0f }, depth, Alignment::Center);
                    }
                }
            };
            render_light_icons(pointLights,  m_PointLightTexture);
            render_light_icons(sunLights,    m_SunLightTexture);
            render_light_icons(spotLights,   m_SpotLightTexture);
            render_light_icons(rodLights,    m_RodLightTexture);
        }
    }
}

namespace Engine {
    [[nodiscard]] Engine::priv::EditorCore& getEditor() noexcept { 
        return *Engine::priv::EditorCore::EDITOR;
    }
}