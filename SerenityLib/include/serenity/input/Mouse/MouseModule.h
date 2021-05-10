#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <serenity/input/Mouse/MouseButtons.h>
#include <serenity/system/TypeDefs.h>
#include <array>

namespace Engine::priv {
    class MouseModule final {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_MouseStatus            = { 0 };
            uint32_t                               m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
            uint32_t                               m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
            uint32_t                               m_NumPressedMouseButtons = 0;
        public:
            void onButtonPressed(uint32_t button) noexcept;
            void onButtonReleased(uint32_t button) noexcept;
            void postUpdate() noexcept;
            void onClearEvents() noexcept;
            [[nodiscard]] inline constexpr MouseButton::Button getCurrentPressedButton() const noexcept { return (MouseButton::Button)m_CurrentMouseButton; }
            [[nodiscard]] inline constexpr uint32_t getNumPressedButtons() const noexcept { return m_NumPressedMouseButtons; }
            [[nodiscard]] inline constexpr bool isButtonDown(uint32_t button) noexcept { return m_MouseStatus[button]; }
            [[nodiscard]] bool isButtonDownOnce(uint32_t button) noexcept;
    };
}

#endif