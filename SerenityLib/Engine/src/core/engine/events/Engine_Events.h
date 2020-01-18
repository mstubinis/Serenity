#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Engine_Window;

#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/events/Engine_EventIncludes.h>

#include <string>
#include <glm/vec2.hpp>
#include <unordered_set>

namespace Engine{
    namespace epriv{
        class EventManager final{
            public:
                static EventManager*                     m_EventManager;
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
    //keyboard functions
    const unsigned int getNumPressedKeys();
    const std::unordered_set<unsigned int>& getPressedKeys();
    const std::unordered_set<unsigned int>& getPressedMouseButtons();

    const bool isKeyDown(const KeyboardKey::Key&);
    const bool isKeyDownOnce(const KeyboardKey::Key&);
    const bool isKeyDownOnce(const KeyboardKey::Key& first, const KeyboardKey::Key& second);
    const bool isKeyDownOnce(const KeyboardKey::Key& first, const KeyboardKey::Key& second, const KeyboardKey::Key& third);
    const bool isKeyDownOnce();

    const bool isKeyUp(const KeyboardKey::Key);


    const KeyboardKey::Key getPressedKey();
    const MouseButton::Button getPressedButton();

    //mouse functions
    const bool isMouseButtonDown(const MouseButton::Button&);
    const bool isMouseButtonDownOnce(const MouseButton::Button&);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();

    const glm::vec2& getMouseDifference(Engine_Window&);
    const glm::vec2& getMousePositionPrevious(Engine_Window&);
    const glm::vec2& getMousePosition(Engine_Window&);

    const double     getMouseWheelDelta(Engine_Window&);
    const double     getMouseWheelDelta();

    void setMousePosition(const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const unsigned int x, const unsigned int y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);

    void setMousePosition(Engine_Window&, const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Engine_Window&, const unsigned int x, const unsigned int y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Engine_Window&, const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(Engine_Window&, const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
};
#endif