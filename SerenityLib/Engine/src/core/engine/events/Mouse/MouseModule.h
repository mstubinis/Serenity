#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <core/engine/events/Mouse/MouseButtons.h>
#include <array>

namespace Engine::priv {
    class MouseModule {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_MouseStatus;
            unsigned int                           m_CurrentMouseButton     = static_cast<unsigned int>(MouseButton::Unknown);
            unsigned int                           m_PreviousMouseButton    = static_cast<unsigned int>(MouseButton::Unknown);
            unsigned int                           m_NumPressedMouseButtons = 0U;
        public:
            MouseModule();
            virtual ~MouseModule();

            void onButtonPressed(unsigned int button);
            void onButtonReleased(unsigned int button);
            void onPostUpdate();
            void onClearEvents();

            MouseButton::Button getCurrentPressedButton() const;

            unsigned int getNumPressedButtons() const;

            bool isButtonDown(unsigned int button);
            bool isButtonDownOnce(unsigned int button);
    };
}

#endif