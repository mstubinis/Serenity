#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <string>
#include <unordered_map>
#include <memory>
#include "Engine_Math.h"
#include "Engine_EventEnums.h"
#include "Engine_Utils.h"

namespace Engine{
    namespace epriv{
        class EventManager final: private Engine::epriv::noncopyable{
            private:
                class impl;
            public:
                std::unique_ptr<impl> m_i;

                EventManager(const char* name,uint w,uint h);
                ~EventManager();

                void _init(const char* name,uint w,uint h);

                void _onEventKeyPressed(uint& key);
                void _onEventKeyReleased(uint& key);
                void _onEventMouseButtonPressed(uint mouseButton);
                void _onEventMouseButtonReleased(uint mouseButton);
                void _onEventMouseWheelMoved(int& delta);
                void _onResetEvents();
                void _setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition);

                void _update(const float& dt);
        };
    };

    //keyboard functions
    bool isKeyDown(std::string str);
    bool isKeyDownOnce(std::string str);
    bool isKeyUp(std::string str);
    bool isKeyDown(KeyboardKey::Key);
    bool isKeyDownOnce(KeyboardKey::Key);
    bool isKeyUp(KeyboardKey::Key);

    //mouse functions
    bool isMouseButtonDown(std::string str);
    bool isMouseButtonDownOnce(std::string str);
    bool isMouseButtonDown(MouseButton::Button);
    bool isMouseButtonDownOnce(MouseButton::Button);

    const glm::vec2& getMouseDifference();
    const glm::vec2& getMousePositionPrevious();
    const glm::vec2& getMousePosition();
    const float getMouseWheelDelta();
    void setMousePosition(float x,float y,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(uint x,uint y,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(glm::vec2,bool resetDifference = false,bool resetPreviousPosition = false);
    void setMousePosition(glm::uvec2,bool resetDifference = false,bool resetPreviousPosition = false);
    const glm::vec2& getMousePosition();
};


#endif