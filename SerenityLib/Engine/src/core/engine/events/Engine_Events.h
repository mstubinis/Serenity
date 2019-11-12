#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/events/Engine_EventIncludes.h>

#include <string>
#include <glm/vec2.hpp>
#include <unordered_map>

namespace Engine{
    namespace epriv{
        class EventManager final{
            public:
                static EventManager*                     m_EventManager;

                std::unordered_map<unsigned int, bool>   m_KeyStatus;
                std::unordered_map<unsigned int, bool>   m_MouseStatus;
                double                                   m_Delta;
                glm::vec2                                m_Position;
                glm::vec2                                m_Position_Previous;
                glm::vec2                                m_Difference;

                unsigned int                             m_currentKey;
                unsigned int                             m_previousKey;

                unsigned int                             m_currentButton;
                unsigned int                             m_previousButton;

                EventDispatcher                          m_EventDispatcher;



                EventManager();
                ~EventManager();

                void setMousePositionInternal(const float x, const float y, const bool resetDifference, const bool resetPrevious);
                void onEventKeyPressed(const unsigned int& key);
                void onEventKeyReleased(const unsigned int& key);
                void onEventMouseButtonPressed(const unsigned int& mouseButton);
                void onEventMouseButtonReleased(const unsigned int& mouseButton);
                void onEventMouseWheelMoved(const int& delta);
                void onResetEvents(const double& dt);
        };
    };
    //keyboard functions
    const bool isKeyDown(const KeyboardKey::Key);
    const bool isKeyDownOnce(const KeyboardKey::Key);
    const bool isKeyDownOnce();
    const bool isKeyUp(const KeyboardKey::Key);

    const bool isKeyDownOnce(const KeyboardKey::Key first, const KeyboardKey::Key second);

    const KeyboardKey::Key getPressedKey();
    const MouseButton::Button getPressedButton();

    //mouse functions
    const bool isMouseButtonDown(const MouseButton::Button);
    const bool isMouseButtonDownOnce(const MouseButton::Button);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();
    const double     getMouseWheelDelta();

    void setMousePosition(const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const unsigned int x, const unsigned int y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
};
#endif