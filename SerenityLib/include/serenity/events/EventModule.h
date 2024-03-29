#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Window;

#include <serenity/events/EventDispatcher.h>
#include <serenity/events/EventIncludes.h>

#include <serenity/input/Keyboard/KeyboardModule.h>
#include <serenity/input/Mouse/MouseModule.h>
#include <serenity/input/Joystick/JoystickModule.h>
#include <serenity/dependencies/glm.h>

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

            void onEventKeyPressed(uint32_t key);
            void onEventKeyReleased(uint32_t key);
            void onEventMouseButtonPressed(uint32_t mouseButton);
            void onEventMouseButtonReleased(uint32_t mouseButton);

            void postUpdate();
            void onWindowLostFocus();
            void onWindowGainedFocus();
        };
};
namespace Engine{
    [[nodiscard]] uint32_t getNumPressedKeys();
    [[nodiscard]] uint32_t getNumPressedMouseButtons();

    [[nodiscard]] bool isKeyDown(KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey, KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey, KeyboardKey, KeyboardKey);
    [[nodiscard]] bool isKeyUp(KeyboardKey);

    [[nodiscard]] bool isCapsLockOn();
    [[nodiscard]] bool isNumLockOn();
    [[nodiscard]] bool isScrollLockOn();

    [[nodiscard]] bool isMouseButtonDown(MouseButton);
    [[nodiscard]] bool isMouseButtonDownOnce(MouseButton);

    [[nodiscard]] const glm::vec2& getMouseDifference();
    [[nodiscard]] const glm::vec2& getMousePositionPrevious();
    [[nodiscard]] const glm::vec2& getMousePosition();

    [[nodiscard]] const glm::vec2& getMouseDifference(Window&);
    [[nodiscard]] const glm::vec2& getMousePositionPrevious(Window&);
    [[nodiscard]] const glm::vec2& getMousePosition(Window&);

    [[nodiscard]] double getMouseWheelDelta(Window&);
    [[nodiscard]] double getMouseWheelDelta();

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