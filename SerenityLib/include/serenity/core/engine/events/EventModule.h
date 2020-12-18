#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Window;

#include <serenity/core/engine/events/EventDispatcher.h>
#include <serenity/core/engine/events/EventIncludes.h>

#include <serenity/core/engine/input/Keyboard/KeyboardModule.h>
#include <serenity/core/engine/input/Mouse/MouseModule.h>
#include <serenity/core/engine/input/Joystick/JoystickModule.h>
#include <serenity/core/engine/dependencies/glm.h>

namespace Engine::priv {
    class EventModule final {
        public:
            EventDispatcher   m_EventDispatcher;

            KeyboardModule    m_KeyboardModule;
            MouseModule       m_MouseModule;
            JoystickModule    m_JoystickModule;

            EventModule();
            EventModule(const EventModule&)                = delete;
            EventModule& operator=(const EventModule&)     = delete;
            EventModule(EventModule&&) noexcept            = delete;
            EventModule& operator=(EventModule&&) noexcept = delete;
            ~EventModule() = default;

            void onEventKeyPressed(uint32_t key);
            void onEventKeyReleased(uint32_t key);
            void onEventMouseButtonPressed(uint32_t mouseButton);
            void onEventMouseButtonReleased(uint32_t mouseButton);

            void postUpdate();
            void onClearEvents();
        };
};
namespace Engine{
    uint32_t getNumPressedKeys();
    uint32_t getNumPressedMouseButtons();

    bool isKeyDown(KeyboardKey::Key key);
    bool isKeyDownOnce(KeyboardKey::Key key);
    bool isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second);
    bool isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second, KeyboardKey::Key third);
    bool isKeyDownOnce();

    bool isKeyUp(KeyboardKey::Key key);

    KeyboardKey::Key getPressedKey();
    MouseButton::Button getPressedButton();

    bool isMouseButtonDown(MouseButton::Button);
    bool isMouseButtonDownOnce(MouseButton::Button);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();

    const glm::vec2& getMouseDifference(Window&);
    const glm::vec2& getMousePositionPrevious(Window&);
    const glm::vec2& getMousePosition(Window&);

    double getMouseWheelDelta(Window&);
    double getMouseWheelDelta();

    void setMousePosition(float x, float y,       bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&,       bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&,      bool resetDifference = false, bool resetPreviousPosition = false);

    void setMousePosition(Window&, float x, float y,       bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, const glm::vec2&,       bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, const glm::uvec2&,      bool resetDifference = false, bool resetPreviousPosition = false);
};
#endif