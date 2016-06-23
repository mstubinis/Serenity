#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

typedef unsigned int uint;

namespace Engine{namespace Events{
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

                static bool _IsMouseButtonDown(std::string str);
                static bool _IsMouseButtonDownOnce(std::string str);
				static void _SetMousePositionInternal(float x, float y);
        };
        glm::vec2 getMouseDifference();
        glm::vec2 getMousePositionPrevious();
        glm::vec2 getMousePosition();
		float getMouseWheelDelta();
        bool isMouseButtonDown(std::string str);
        bool isMouseButtonDownOnce(std::string str);

		void setMousePosition(float x,float y);
		void setMousePosition(glm::vec2);
		void setMousePosition(glm::uvec2);
    };
    namespace Keyboard{
        class KeyProcessing final{
            private:
                static std::unordered_map<std::string,uint> m_KeyMap;
            public:
                static uint m_currentKey;
                static uint m_previousKey;
                static std::unordered_map<uint,bool> m_KeyStatus;

                static bool _IsKeyDown(std::string str);
                static bool _IsKeyUp(std::string str);
                static bool _IsKeyDownOnce(std::string str);
        };
        bool isKeyDown(std::string str);
        bool isKeyDownOnce(std::string str);
        bool isKeyUp(std::string str);
    };
    bool isKeyDown(std::string str);
    bool isKeyDownOnce(std::string str);
    bool isKeyUp(std::string str);

    glm::vec2 getMousePosition();
};    };
#endif