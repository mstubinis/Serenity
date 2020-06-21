#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Window;

#include <core/engine/events/EventDispatcher.h>
#include <core/engine/events/EventIncludes.h>

#include <core/engine/events/Keyboard/KeyboardModule.h>
#include <core/engine/events/Mouse/MouseModule.h>
#include <core/engine/events/Joystick/JoystickModule.h>

#include <string>
#include <glm/vec2.hpp>
#include <unordered_set>

namespace Engine::priv {
    class EventModule final : public Engine::NonCopyable, Engine::NonMoveable {
        public:
            EventDispatcher   m_EventDispatcher;

            KeyboardModule    m_KeyboardModule;
            MouseModule       m_MouseModule;
            JoystickModule    m_JoystickModule;

            EventModule();
            ~EventModule();

            void onEventKeyPressed(unsigned int key);
            void onEventKeyReleased(unsigned int key);
            void onEventMouseButtonPressed(unsigned int mouseButton);
            void onEventMouseButtonReleased(unsigned int mouseButton);

            void onPostUpdate();
            void onClearEvents();
        };
};
namespace Engine{
    unsigned int getNumPressedKeys();
    unsigned int getNumPressedMouseButtons();

    bool isKeyDown(KeyboardKey::Key key);
    bool isKeyDownOnce(KeyboardKey::Key key);
    bool isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second);
    bool isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second, KeyboardKey::Key third);
    bool isKeyDownOnce();

    bool isKeyUp(KeyboardKey::Key key);


    KeyboardKey::Key getPressedKey();
    MouseButton::Button getPressedButton();

    bool isMouseButtonDown(MouseButton::Button mouseButton);
    bool isMouseButtonDownOnce(MouseButton::Button mouseButton);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();

    const glm::vec2& getMouseDifference(Window& window);
    const glm::vec2& getMousePositionPrevious(Window& window);
    const glm::vec2& getMousePosition(Window& window);

    double getMouseWheelDelta(Window& window);
    double getMouseWheelDelta();

    void setMousePosition(float x, float y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(unsigned int x, unsigned int y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, bool resetDifference = false, bool resetPreviousPosition = false);

    void setMousePosition(Window& window, float x, float y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window& window, unsigned int x, unsigned int y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window& window, const glm::vec2&, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window& window, const glm::uvec2&, bool resetDifference = false, bool resetPreviousPosition = false);
};
#endif