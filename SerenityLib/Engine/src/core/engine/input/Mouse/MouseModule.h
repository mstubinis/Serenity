#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <core/engine/input/Mouse/MouseButtons.h>

namespace Engine::priv {
    class MouseModule {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_MouseStatus;
            uint32_t                               m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
            uint32_t                               m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
            uint32_t                               m_NumPressedMouseButtons = 0U;
        public:
            MouseModule() {
                m_MouseStatus.fill(false);
            }
            virtual ~MouseModule() = default;

            CONSTEXPR void onButtonPressed(uint32_t button) noexcept {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = m_CurrentMouseButton;
                m_CurrentMouseButton  = button;
                if (!m_MouseStatus[button]) {
                    m_MouseStatus[button] = true;
                }
            }
            CONSTEXPR void onButtonReleased(uint32_t button) noexcept {
                if (button == MouseButton::Unknown) {
                    return;
                }
                m_PreviousMouseButton = (uint32_t)MouseButton::Unknown;
                m_CurrentMouseButton  = (uint32_t)MouseButton::Unknown;
                if (m_MouseStatus[button]) {
                    m_MouseStatus[button] = false;
                }
            }
            CONSTEXPR void postUpdate() noexcept {
                m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
                m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            void onClearEvents() noexcept {
                m_MouseStatus.fill(false);
                m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
                m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
                m_NumPressedMouseButtons = 0U;
            }
            CONSTEXPR MouseButton::Button getCurrentPressedButton() const noexcept {
                return (MouseButton::Button)m_CurrentMouseButton;
            }
            CONSTEXPR uint32_t getNumPressedButtons() const noexcept {
                return m_NumPressedMouseButtons;
            }
            CONSTEXPR bool isButtonDown(uint32_t button) noexcept {
                return (m_MouseStatus[button] == true);
            }
            CONSTEXPR bool isButtonDownOnce(uint32_t button) noexcept {
                const bool res = isButtonDown(button);
                return res && m_CurrentMouseButton == button && m_CurrentMouseButton != m_PreviousMouseButton;
            }
    };
}

#endif