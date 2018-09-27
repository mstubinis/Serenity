#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <string>
#include <glm/vec2.hpp>
#include "Engine_EventEnums.h"
#include "Engine_Utils.h"
#include <unordered_map>

namespace Engine{
    namespace epriv{
        class EventManager final: private Engine::epriv::noncopyable{
            public:
                static EventManager* m_EventManager;

                std::unordered_map<uint, bool> m_KeyStatus, m_MouseStatus;
                float m_Delta;
                glm::vec2 m_Position, m_Position_Previous, m_Difference;
                uint m_currentKey, m_previousKey, m_currentButton, m_previousButton;
                void setMousePositionInternal(float x, float y, bool resetDifference, bool resetPrevious);

 
                EventManager(const char* name,uint w,uint h);
                ~EventManager();

                void onEventKeyPressed(uint& key);
                void onEventKeyReleased(uint& key);
                void onEventMouseButtonPressed(uint& mouseButton);
                void onEventMouseButtonReleased(uint& mouseButton);
                void onEventMouseWheelMoved(int& delta);
                void onResetEvents();
        };
    };
    //keyboard functions
    const bool isKeyDown(KeyboardKey::Key);
    const bool isKeyDownOnce(KeyboardKey::Key);
    const bool isKeyUp(KeyboardKey::Key);

    //mouse functions
    const bool isMouseButtonDown(MouseButton::Button);
    const bool isMouseButtonDownOnce(MouseButton::Button);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();
    const float getMouseWheelDelta();

    void setMousePosition(float x,float y,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(uint x,uint y,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(glm::vec2,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(glm::uvec2,bool resetDifference = false,bool resetPreviousPosition = false);
};
#endif