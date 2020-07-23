#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <core/engine/input/Mouse/MouseButtons.h>

namespace Engine::priv {
    class MouseModule {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_MouseStatus;
            unsigned int                           m_CurrentMouseButton     = (unsigned int)MouseButton::Unknown;
            unsigned int                           m_PreviousMouseButton    = (unsigned int)MouseButton::Unknown;
            unsigned int                           m_NumPressedMouseButtons = 0U;
        public:
            MouseModule() {
                m_MouseStatus.fill(false);
            }
            virtual ~MouseModule() = default;

            CONSTEXPR void onButtonPressed(unsigned int button) noexcept {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = m_CurrentMouseButton;
                m_CurrentMouseButton  = button;
                if (!m_MouseStatus[button]) {
                    m_MouseStatus[button] = true;
                }
            }
            CONSTEXPR void onButtonReleased(unsigned int button) noexcept {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton  = (unsigned int)MouseButton::Unknown;
                if (m_MouseStatus[button]) {
                    m_MouseStatus[button] = false;
                }
            }
            CONSTEXPR void onPostUpdate() noexcept {
                m_PreviousMouseButton    = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton     = (unsigned int)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            void onClearEvents() noexcept {
                m_MouseStatus.fill(false);
                m_PreviousMouseButton    = (unsigned int)MouseButton::Unknown;
                m_CurrentMouseButton     = (unsigned int)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            CONSTEXPR MouseButton::Button getCurrentPressedButton() const noexcept {
                return (MouseButton::Button)m_CurrentMouseButton;
            }
            CONSTEXPR unsigned int getNumPressedButtons() const noexcept {
                return m_NumPressedMouseButtons;
            }
            CONSTEXPR bool isButtonDown(unsigned int button) noexcept {
                return (m_MouseStatus[button] == true);
            }
            CONSTEXPR bool isButtonDownOnce(unsigned int button) noexcept {
                const bool res = isButtonDown(button);
                return res && m_CurrentMouseButton == button && m_CurrentMouseButton != m_PreviousMouseButton;
            }
    };
}

#endif