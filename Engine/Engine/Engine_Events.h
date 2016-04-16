#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

typedef unsigned int uint;

namespace Engine{
    namespace Events{
        namespace Mouse{
            class MouseProcessing final{
                private:
                    static std::unordered_map<std::string,uint> m_MouseMap;
                public:
                    static float m_Delta;
                    static uint m_currentButton;
                    static uint m_previousButton;
                    static std::unordered_map<uint,bool> m_MouseStatus;

                    static glm::vec2 m_Position, m_Position_Previous,m_Difference;

                    static bool MouseProcessing::_IsMouseButtonDown(std::string str);
                    static bool MouseProcessing::_IsMouseButtonDownOnce(std::string str);
            };
            static glm::vec2 getMouseDifference(){ return MouseProcessing::m_Difference; }
            static glm::vec2 getMousePositionPrevious(){ return MouseProcessing::m_Position_Previous; }
            static glm::vec2 getMousePosition(){ return MouseProcessing::m_Position; }
            static bool isMouseButtonDown(std::string str){ return MouseProcessing::_IsMouseButtonDown(str); }
            static bool isMouseButtonDownOnce(std::string str){ return MouseProcessing::_IsMouseButtonDownOnce(str); }
            static float getMouseWheelDelta(){ return MouseProcessing::m_Delta; }
        };
        namespace Keyboard{
            class KeyProcessing final{
                private:
                    static std::unordered_map<std::string,uint> m_KeyMap;
                public:
                    static uint m_currentKey;
                    static uint m_previousKey;
                    static std::unordered_map<uint,bool> m_KeyStatus;

                    static bool KeyProcessing::_IsKeyDown(std::string str);
                    static bool KeyProcessing::_IsKeyUp(std::string str);
                    static bool KeyProcessing::_IsKeyDownOnce(std::string str);
            };
            static bool isKeyDown(std::string str){ return KeyProcessing::_IsKeyDown(str); }
            static bool isKeyDownOnce(std::string str){ return KeyProcessing::_IsKeyDownOnce(str); }
            static bool isKeyUp(std::string str){ return KeyProcessing::_IsKeyUp(str); }
        };
        static bool isKeyDown(std::string str){ return Keyboard::KeyProcessing::_IsKeyDown(str); }
        static bool isKeyDownOnce(std::string str){ return Keyboard::KeyProcessing::_IsKeyDownOnce(str); }
        static bool isKeyUp(std::string str){ return Keyboard::KeyProcessing::_IsKeyUp(str); }

        static glm::vec2 getMousePosition(){ return Mouse::MouseProcessing::m_Position; }
        static bool isMouseButtonDown(std::string str){ return Mouse::MouseProcessing::_IsMouseButtonDown(str); }
        static bool isMouseButtonDownOnce(std::string str){ return Mouse::MouseProcessing::_IsMouseButtonDownOnce(str); }
        static float getMouseWheelDelta(){ return Mouse::MouseProcessing::m_Delta; }
    };
};
#endif