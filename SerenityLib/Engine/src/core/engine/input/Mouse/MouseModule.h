#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <core/engine/input/Mouse/MouseButtons.h>

namespace Engine::priv {
    class MouseModule {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_MouseStatus;
            unsigned int                           m_CurrentMouseButton = static_cast<unsigned int>(MouseButton::Unknown);
            unsigned int                           m_PreviousMouseButton = static_cast<unsigned int>(MouseButton::Unknown);
            unsigned int                           m_NumPressedMouseButtons = 0U;
        public:
            MouseModule() {
                m_MouseStatus.fill(false);
            }
            virtual ~MouseModule() {}

            constexpr void onButtonPressed(unsigned int button) {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = m_CurrentMouseButton;
                m_CurrentMouseButton  = button;
                if (!m_MouseStatus[button]) {
                    m_MouseStatus[button] = true;
                }
            }
            constexpr void onButtonReleased(unsigned int button) {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton  = (unsigned int)MouseButton::Unknown;
                if (m_MouseStatus[button]) {
                    m_MouseStatus[button] = false;
                }
            }
            constexpr void onPostUpdate() {
                m_PreviousMouseButton    = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton     = (unsigned int)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            void onClearEvents() {
                m_MouseStatus.fill(false);
                m_PreviousMouseButton    = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton     = (unsigned int)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            constexpr MouseButton::Button getCurrentPressedButton() const {
                return (MouseButton::Button)m_CurrentMouseButton;
            }
            constexpr unsigned int getNumPressedButtons() const {
                return m_NumPressedMouseButtons;
            }
            constexpr bool isButtonDown(unsigned int button) {
                return (m_MouseStatus[button] == true);
            }
            constexpr bool isButtonDownOnce(unsigned int button) {
                const bool res = isButtonDown(button);
                return res && m_CurrentMouseButton == button && m_CurrentMouseButton != m_PreviousMouseButton;
            }
    };
}

#endif