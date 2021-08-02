#pragma once
#ifndef ENGINE_EVENTS_MOUSE_MODULE_H
#define ENGINE_EVENTS_MOUSE_MODULE_H

#include <serenity/input/Mouse/MouseButtons.h>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class MouseModule final {
        using MouseStatusArray = std::vector<bool>;
        private:
            MouseStatusArray  m_PrevMouseStatus        = MouseStatusArray(MouseButton::_TOTAL, false);
            MouseStatusArray  m_CurrMouseStatus        = MouseStatusArray(MouseButton::_TOTAL, false);
            int32_t           m_NumPressedMouseButtons = 0;
        public:
            void onWindowLostFocus() noexcept;
            void onWindowGainedFocus() noexcept;
            void onButtonPressed(uint32_t button) noexcept;
            void onButtonReleased(uint32_t button) noexcept;
            void update() noexcept;
            [[nodiscard]] inline constexpr int32_t getNumPressedButtons() const noexcept { return m_NumPressedMouseButtons; }
            [[nodiscard]] inline constexpr bool isButtonDown(uint32_t button) noexcept { return m_CurrMouseStatus[button]; }
            [[nodiscard]] bool isButtonDownOnce(uint32_t button) noexcept;
    };
}

#endif