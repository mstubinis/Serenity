#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <serenity/input/Mouse/MouseButtons.h>
#include <serenity/system/TypeDefs.h>
#include <array>

namespace Engine::priv {
    class MouseModule final {
        private:
            std::array<bool, MouseButton::_TOTAL>  m_PrevMouseStatus        = { 0 };
            std::array<bool, MouseButton::_TOTAL>  m_CurrMouseStatus        = { 0 };
            uint32_t                               m_NumPressedMouseButtons = 0;
        public:
            void onWindowLostFocus() noexcept;
            void onWindowGainedFocus() noexcept;
            void onButtonPressed(uint32_t button) noexcept;
            void onButtonReleased(uint32_t button) noexcept;
            void update() noexcept;
            [[nodiscard]] inline constexpr uint32_t getNumPressedButtons() const noexcept { return m_NumPressedMouseButtons; }
            [[nodiscard]] inline constexpr bool isButtonDown(uint32_t button) noexcept { return m_CurrMouseStatus[button]; }
            [[nodiscard]] bool isButtonDownOnce(uint32_t button) noexcept;
    };
}

#endif