#include <serenity/editor/core/EditorCore.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/resources/Engine_Resources.h>
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

Engine::view_ptr<Engine::priv::EditorCore> Engine::priv::EditorCore::EDITOR;

Engine::priv::EditorCore::EditorCore(const EngineOptions& options) {
    m_Enabled = options.editor_enabled;
    if (m_Enabled) {

    }
    EDITOR = this;
}
Engine::priv::EditorCore::~EditorCore() {
    if (m_Enabled) {
        ImGui::SFML::Shutdown();
        m_RegisteredWindows.clear();
    }
}
bool Engine::priv::EditorCore::isWindowRegistered(Window& window) const noexcept {
    return m_RegisteredWindows.contains(&window);
}
void Engine::priv::EditorCore::processEvent(const sf::Event& e) {
    if (m_Enabled && m_Shown) {
        ImGui::SFML::ProcessEvent(e);
    }
}
void Engine::priv::EditorCore::init(const EngineOptions& options, Engine::priv::ResourceManager& resourceManager) {
    if (m_Enabled) {
        ImGui::SFML::Init(resourceManager.m_Windows[0]->getSFMLHandle(), static_cast<sf::Vector2f>(resourceManager.m_Windows[0]->getSFMLHandle().getSize()));
        ImGui_ImplOpenGL3_Init();
        m_RegisteredWindows.insert(resourceManager.m_Windows[0].get());

        m_PointLightTexture = internal_load_embedded_image(POINT_LIGHT_IMAGE_DATA, 32, 32, "PointLightEditorTexture");
        m_SunLightTexture   = internal_load_embedded_image(SUN_LIGHT_IMAGE_DATA, 32, 32, "SunLightEditorTexture");
        m_SpotLightTexture  = internal_load_embedded_image(SPOT_LIGHT_IMAGE_DATA, 32, 32, "SpotLightEditorTexture");
    }
}
void Engine::priv::EditorCore::update(Window& window, const float dt) {
    if (m_Enabled && m_Shown && isWindowRegistered(window)) {
        sf::Time t(sf::seconds(dt));
        ImGui::SFML::Update(window.getSFMLHandle(), t);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        Engine::priv::EditorWindowScene windowScene;
        windowScene.update();

        ImGui::EndFrame();
    }
}
void Engine::priv::EditorCore::render(Window& window) {
    if (m_Enabled && m_Shown && isWindowRegistered(window)) {
        //ImGui::SFML::Render(window.getSFMLHandle());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
Handle Engine::priv::EditorCore::internal_load_embedded_image(const uint8_t* data, int width, int height, const char* textureName) {
    sf::Image sfimg;
    sfimg.create(width, height);
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            const auto pixel = data[(row * width) + col];
            if (pixel == 128) {
                sfimg.setPixel(col, row, sf::Color(0, 0, 0, 0));
            } else if (pixel == 0) {
                sfimg.setPixel(col, row, sf::Color(0, 0, 0, 255));
            } else if (pixel == 255) {
                sfimg.setPixel(col, row, sf::Color(255, 255, 255, 255));
            } else {
                sfimg.setPixel(col, row, sf::Color(128, 128, 128, 255));
            }
        }
    }
    return Engine::Resources::loadTexture(sfimg, textureName);
}
void Engine::priv::EditorCore::renderLightIcons(Scene& scene) {
    if (m_Enabled && m_Shown) {
        auto& directionalLights = Engine::priv::PublicScene::GetLights<DirectionalLight>(scene);
        auto& sunLights         = Engine::priv::PublicScene::GetLights<SunLight>(scene);
        auto& pointLights       = Engine::priv::PublicScene::GetLights<PointLight>(scene);
        auto& spotLights        = Engine::priv::PublicScene::GetLights<SpotLight>(scene);
        auto& rodLights         = Engine::priv::PublicScene::GetLights<RodLight>(scene);
        auto camera             = scene.getActiveCamera();
        if (camera) {
            auto render_light_icons = [&camera](auto& container, Handle texture) {
                const auto white = glm::vec4{ 1.0f };
                const auto depth = 0.1f;
                for (const auto& light : container) {
                    const auto twoDPos = Engine::Math::getScreenCoordinates(glm::vec3{ light->getPosition() }, *camera, false);
                    if (twoDPos.z > 0) {
                        Engine::Renderer::renderTexture(texture, glm::vec2{ twoDPos }, white, 0.0f, glm::vec2{ 1.0f }, depth);
                    }
                }
            };
            render_light_icons(pointLights,  m_PointLightTexture);
            render_light_icons(sunLights,    m_SunLightTexture);
            render_light_icons(spotLights,   m_SpotLightTexture);
        }
    }
}