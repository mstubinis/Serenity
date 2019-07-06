#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/events/Engine_EventEnums.h>

#include <string>
#include <glm/vec2.hpp>
#include <unordered_map>

namespace Engine{
    namespace epriv{
        class EventManager final{
            public:
                static EventManager*             m_EventManager;

                std::unordered_map<uint, bool>   m_KeyStatus;
                std::unordered_map<uint, bool>   m_MouseStatus;
                double                           m_Delta;
                glm::vec2                        m_Position;
                glm::vec2                        m_Position_Previous;
                glm::vec2                        m_Difference;

                uint                             m_currentKey;
                uint                             m_previousKey;
                uint                             m_currentButton;
                uint                             m_previousButton;

                EventDispatcher                  m_EventDispatcher;



                EventManager(const char* name, const uint w, const uint h);
                ~EventManager();

                void setMousePositionInternal(const float x, const float y, const bool resetDifference, const bool resetPrevious);
                void onEventKeyPressed(const uint& key);
                void onEventKeyReleased(const uint& key);
                void onEventMouseButtonPressed(const uint& mouseButton);
                void onEventMouseButtonReleased(const uint& mouseButton);
                void onEventMouseWheelMoved(const int& delta);
                void onResetEvents(const double& dt);
        };
    };
    //keyboard functions
    const bool isKeyDown(const KeyboardKey::Key);
    const bool isKeyDownOnce(const KeyboardKey::Key);
    const bool isKeyUp(const KeyboardKey::Key);

    const bool isKeyDownOnce(const KeyboardKey::Key first, const KeyboardKey::Key second);


    //mouse functions
    const bool isMouseButtonDown(const MouseButton::Button);
    const bool isMouseButtonDownOnce(const MouseButton::Button);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();
    const double getMouseWheelDelta();

    void setMousePosition(const float x, const float y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const uint x, const uint y, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, const bool resetDifference = false, const bool resetPreviousPosition = false);
};
#endif