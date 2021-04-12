#include <serenity/editor/core/EditorCore.h>
#include <serenity/system/EngineOptions.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/editor/imgui/imgui.h>
#include <serenity/editor/imgui/imgui-SFML.h>
#include <serenity/editor/imgui/imgui_impl_opengl3.h>
#include <serenity/system/window/Window.h>

#include <serenity/editor/windowScene/WindowScene.h>

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