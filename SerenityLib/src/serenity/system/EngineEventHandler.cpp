
#include <serenity/system/EngineEventHandler.h>
#include <serenity/system/EngineGameFunctions.h>
#include <serenity/system/window/Window.h>
#include <serenity/events/Event.h>
#include <serenity/events/EventModule.h>

#include <serenity/resources/Engine_Resources.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/scene/Scene.h>
#include <serenity/ecs/components/ComponentCamera.h>
#include <serenity/editor/core/EditorCore.h>
#include <serenity/system/Engine.h>

Engine::priv::EngineEventHandler::EngineEventHandler(EditorCore& editorCore, EventModule& eventModule, RenderModule& renderModule, ResourceManager& resourceManager)
    : m_EditorCore      { editorCore }
    , m_EventModule     { eventModule }
    , m_RenderModule    { renderModule }
    , m_ResourceManager { resourceManager }
{}

void Engine::priv::EngineEventHandler::internal_dispatch_event(Event&& inEvent) noexcept {
    m_EventModule.m_EventDispatcher.dispatchEvent(std::move(inEvent));
}

void Engine::priv::EngineEventHandler::internal_on_event_resize(Window& window, uint32_t newWindowWidth, uint32_t newWindowHeight, bool saveSize, GameCore& gameCore) {
    m_RenderModule._resize(newWindowWidth, newWindowHeight);
    window.m_Data.internal_on_resize(newWindowWidth, newWindowHeight, saveSize);
    //resize cameras and viewports here
    for (auto& scene : m_ResourceManager.scenes()) {
        if (scene) {
            scene->onResize(newWindowWidth, newWindowHeight);
            PublicScene::GetECS(*scene).onResize<ComponentCamera>(newWindowWidth, newWindowHeight);
            for (auto& viewport : PublicScene::GetViewports(*scene)) {
                if (viewport.m_ResizeFuncPointer) {
                    viewport.m_ResizeFuncPointer(0.0f, 0.0f, float(newWindowWidth), float(newWindowHeight), viewport, viewport.m_ResizeFuncPointerUserData);
                }
            }
        }
    }
    Game::onResize(window, newWindowWidth, newWindowHeight, gameCore);
    internal_dispatch_event(Event{ EventType::WindowResized, EventWindowResized{ newWindowWidth, newWindowHeight } });
}
void Engine::priv::EngineEventHandler::internal_on_event_game_ended() {
    Game::onGameEnded();
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::GameEnded);
}
void Engine::priv::EngineEventHandler::internal_on_event_window_closed(Window& window) {
    Game::onWindowClosed(window);
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowHasClosed);
    window.m_Data.internal_on_close();
    Engine::stop();
}
void Engine::priv::EngineEventHandler::internal_on_event_window_requested_closed(Window& window) {
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowRequestedToBeClosed);
    Game::onWindowRequestedToBeClosed(window);
    window.close();
}
void Engine::priv::EngineEventHandler::internal_on_event_lost_focus(Window& window, GameCore& gameCore) {
    m_EventModule.onWindowLostFocus();
    Game::onLostFocus(window, gameCore);
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowLostFocus);
}
void Engine::priv::EngineEventHandler::internal_on_event_gained_focus(Window& window, GameCore& gameCore) {
    m_EventModule.onWindowGainedFocus();
    Game::onGainedFocus(window, gameCore);
    window.m_Data.internal_update_on_reset_events(0.0f);
    m_EventModule.m_EventDispatcher.dispatchEvent(EventType::WindowGainedFocus);
}
void Engine::priv::EngineEventHandler::internal_on_event_text_entered(Window& window, uint32_t unicode) {
    Game::onTextEntered(window, unicode);
    internal_dispatch_event(Event{ EventType::TextEntered, EventTextEntered{ unicode } });
}
void Engine::priv::EngineEventHandler::internal_on_event_key_pressed(Window& window, uint32_t key) {
    ++key;
    m_EventModule.onEventKeyPressed(key);
    Game::onKeyPressed(window, key);

    EventKeyboard e(key);
    if (Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl)) e.control = true;
    if (Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))     e.alt     = true;
    if (Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))   e.shift   = true;
    if (Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))  e.system  = true;

    internal_dispatch_event(Event{ EventType::KeyPressed, std::move(e) });
}
void Engine::priv::EngineEventHandler::internal_on_event_key_released(Window& window, uint32_t key) {
    ++key;
    m_EventModule.onEventKeyReleased(key);
    Game::onKeyReleased(window, key);

    EventKeyboard e(key);
    if (Engine::isKeyDown(KeyboardKey::LeftControl) || Engine::isKeyDown(KeyboardKey::RightControl)) e.control = true;
    if (Engine::isKeyDown(KeyboardKey::LeftAlt)     || Engine::isKeyDown(KeyboardKey::RightAlt))     e.alt     = true;
    if (Engine::isKeyDown(KeyboardKey::LeftShift)   || Engine::isKeyDown(KeyboardKey::RightShift))   e.shift   = true;
    if (Engine::isKeyDown(KeyboardKey::LeftSystem)  || Engine::isKeyDown(KeyboardKey::RightSystem))  e.system  = true;

    internal_dispatch_event(Event{ EventType::KeyReleased, std::move(e) });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_wheel_scrolled(Window& window, float delta, int mouseWheelX, int mouseWheelY) {
    window.m_Data.internal_on_mouse_wheel_scrolled(delta, mouseWheelX, mouseWheelY);
    Game::onMouseWheelScrolled(window, delta, mouseWheelX, mouseWheelY);
    internal_dispatch_event(Event{ EventType::MouseWheelMoved, EventMouseWheel{ delta, mouseWheelX, mouseWheelY } });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_button_pressed(Window& window, uint32_t mouseButton) {
    ++mouseButton;
    m_EventModule.onEventMouseButtonPressed(mouseButton);
    Game::onMouseButtonPressed(window, mouseButton);
    internal_dispatch_event(Event{ EventType::MouseButtonPressed, EventMouseButton( mouseButton, window ) });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_button_released(Window& window, uint32_t mouseButton) {
    ++mouseButton;
    m_EventModule.onEventMouseButtonReleased(mouseButton);
    Game::onMouseButtonReleased(window, mouseButton);
    internal_dispatch_event(Event{ EventType::MouseButtonReleased, EventMouseButton( mouseButton, window ) });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_moved(Window& window, int mouseX, int mouseY, GameCore& gameCore) {
    const float mX = float(mouseX);
    const float mY = float(mouseY);
    if (window.hasFocus()) {
        window.updateMousePosition(mX, mY, false, false);
    }
    Game::onMouseMoved(window, mX, mY, gameCore);
    internal_dispatch_event(Event{ EventType::MouseMoved, EventMouseMove{ mX, mY } });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_entered(Window& window, GameCore& gameCore) {
    Game::onMouseEntered(window, gameCore);
    internal_dispatch_event(Event{ EventType::MouseEnteredWindow, EventMouseMove{ window } });
}
void Engine::priv::EngineEventHandler::internal_on_event_mouse_left(Window& window, GameCore& gameCore) {
    Game::onMouseLeft(window, gameCore);
    internal_dispatch_event(Event{ EventType::MouseLeftWindow, EventMouseMove{ window } });
}
void Engine::priv::EngineEventHandler::internal_on_event_joystick_button_pressed(Window& window, uint32_t button, uint32_t id) {
    Game::onJoystickButtonPressed();
    internal_dispatch_event(Event{ EventType::JoystickButtonPressed, EventJoystickButton{ id, button } });
}
void Engine::priv::EngineEventHandler::internal_on_event_joystick_button_released(Window& window, uint32_t button, uint32_t id) {
    Game::onJoystickButtonReleased();
    internal_dispatch_event(Event{ EventType::JoystickButtonReleased, EventJoystickButton{ id, button } });
}
void Engine::priv::EngineEventHandler::internal_on_event_joystick_moved(Window& window, uint32_t id, float position, uint32_t axis) {
    Game::onJoystickMoved();
    internal_dispatch_event(Event{ EventType::JoystickMoved, EventJoystickMoved( id, axis, position ) });
}
void Engine::priv::EngineEventHandler::internal_on_event_joystick_connected(Window& window, uint32_t id) {
    Game::onJoystickConnected();
    internal_dispatch_event(Event{ EventType::JoystickConnected, EventJoystickConnection{ id } });
}
void Engine::priv::EngineEventHandler::internal_on_event_joystick_disconnected(Window& window, uint32_t id) {
    Game::onJoystickDisconnected();
    internal_dispatch_event(Event{ EventType::JoystickDisconnected, EventJoystickConnection{ id } });
}
void Engine::priv::EngineEventHandler::poll_events(Window& window, GameCore& gameCore) {
    sf::Event e;
    while (window.pollEvents(e)) {
        m_EditorCore.processEvent(e);
        switch (e.type) {
            case sf::Event::Closed: {
                internal_on_event_window_requested_closed(window); break;
            } case sf::Event::LostFocus: {
                internal_on_event_lost_focus(window, gameCore); break;
            } case sf::Event::GainedFocus: {
                internal_on_event_gained_focus(window, gameCore); break;
            } case sf::Event::KeyReleased: {
                internal_on_event_key_released(window, e.key.code); break;
            } case sf::Event::KeyPressed: {
                internal_on_event_key_pressed(window, e.key.code); break;
            } case sf::Event::MouseButtonPressed: {
                internal_on_event_mouse_button_pressed(window, e.mouseButton.button); break;
            } case sf::Event::MouseButtonReleased: {
                internal_on_event_mouse_button_released(window, e.mouseButton.button); break;
            } case sf::Event::MouseEntered: {
                internal_on_event_mouse_entered(window, gameCore); break;
            } case sf::Event::MouseLeft: {
                internal_on_event_mouse_left(window, gameCore); break;
            } case sf::Event::MouseWheelScrolled: {
                internal_on_event_mouse_wheel_scrolled(window, e.mouseWheelScroll.delta, e.mouseWheelScroll.x, e.mouseWheelScroll.y); break;
            } case sf::Event::MouseMoved: {
                internal_on_event_mouse_moved(window, e.mouseMove.x, e.mouseMove.y, gameCore); break;
            } case sf::Event::Resized: {
                internal_on_event_resize(window, e.size.width, e.size.height, true, gameCore); break;
            } case sf::Event::TextEntered: {
                internal_on_event_text_entered(window, e.text.unicode); break;
            } case sf::Event::JoystickButtonPressed: {
                internal_on_event_joystick_button_pressed(window, e.joystickButton.button, e.joystickButton.joystickId); break;
            } case sf::Event::JoystickButtonReleased: {
                internal_on_event_joystick_button_released(window, e.joystickButton.button, e.joystickButton.joystickId); break;
            } case sf::Event::JoystickConnected: {
                internal_on_event_joystick_connected(window, e.joystickConnect.joystickId); break;
            } case sf::Event::JoystickDisconnected: {
                internal_on_event_joystick_disconnected(window, e.joystickConnect.joystickId); break;
            } case sf::Event::JoystickMoved: {
                internal_on_event_joystick_moved(window, e.joystickMove.joystickId, e.joystickMove.position, e.joystickMove.axis); break;
            }
        }
    }
}