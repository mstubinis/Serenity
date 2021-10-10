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
            void onButtonPressed(MouseButton button) noexcept;
            void onButtonReleased(MouseButton button) noexcept;
            void postUpdate() noexcept;
            [[nodiscard]] inline int32_t getNumPressedButtons() const noexcept { return m_NumPressedMouseButtons; }
            [[nodiscard]] inline bool isButtonDown(MouseButton button) noexcept { return m_CurrMouseStatus[button]; }
            [[nodiscard]] bool isButtonDownOnce(MouseButton button) noexcept;
    };
}

#endif