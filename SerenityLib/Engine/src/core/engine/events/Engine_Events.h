#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Window;

#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/events/Engine_EventIncludes.h>

#include <string>
#include <glm/vec2.hpp>
#include <unordered_set>

namespace Engine::priv {
    class EventManager final {
        public:
            EventDispatcher                          m_EventDispatcher;

            std::unordered_set<unsigned int>         m_KeyboardKeyStatus;
            std::unordered_set<unsigned int>         m_MouseStatus;

            unsigned int                             m_CurrentKeyboardKey;
            unsigned int                             m_PreviousKeyboardKey;

            unsigned int                             m_CurrentMouseButton;
            unsigned int                             m_PreviousMouseButton;


            EventManager();
            ~EventManager();

            void cleanup();

            void onEventKeyPressed(const unsigned int& key);
            void onEventKeyReleased(const unsigned int& key);
            void onEventMouseButtonPressed(const unsigned int& mouseButton);
            void onEventMouseButtonReleased(const unsigned int& mouseButton);
            void onResetEvents(const double& dt);
        };
};
namespace Engine::events {
    void dispatchEvent(const unsigned int& eventType);
};
namespace Engine{
    const unsigned int getNumPressedKeys();
    const std::unordered_set<unsigned int>& getPressedKeys();
    const std::unordered_set<unsigned int>& getPressedMouseButtons();

    const bool isKeyDown(const KeyboardKey::Key& key);
    const bool isKeyDownOnce(const KeyboardKey::Key& key);
    const bool isKeyDownOnce(const KeyboardKey::Key& first, const KeyboardKey::Key& second);
    const bool isKeyDownOnce(const KeyboardKey::Key& first, const KeyboardKey::Key& second, const KeyboardKey::Key& third);
    const bool isKeyDownOnce();

    const bool isKeyUp(const KeyboardKey::Key& key);


    const KeyboardKey::Key getPressedKey();
    const MouseButton::Button getPressedButton();

    const bool isMouseButtonDown(const MouseButton::Button& mouseButton);
    const bool isMouseButtonDownOnce(const MouseButton::Button& mouseButton);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();

    const glm::vec2& getMouseDifference(Window& window);
    const glm::vec2& getMousePositionPrevious(Window& window);
    const glm::vec2& getMousePosition(Window& window);

    const double     getMouseWheelDelta(Window& window);
    const double     getMouseWheelDelta();

    void setMousePosition(const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const unsigned int x, const unsigned int y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);

    void setMousePosition(Window& window, const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Window& window, const unsigned int x, const unsigned int y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Window& window, const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Window& window, const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
};
#endif