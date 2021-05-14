#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/system/TypeDefs.h>
#include <array>

namespace Engine::priv {
    class KeyboardModule final {
        using KeyStatusArray = std::array<bool, KeyboardKey::_TOTAL>;
        private:
            KeyStatusArray  m_CurrKeyboardKeyStatus = { 0 };
            KeyStatusArray  m_PrevKeyboardKeyStatus = { 0 };
            uint32_t        m_NumPressedKeys        = 0;
        public:
            void onKeyPressed(uint32_t key) noexcept;
            void onKeyReleased(uint32_t key) noexcept;
            void update() noexcept;
            void onClearEvents() noexcept;
            [[nodiscard]] inline constexpr uint32_t getNumPressedKeys() const noexcept { return m_NumPressedKeys; }
            [[nodiscard]] inline constexpr bool isKeyDown(uint32_t key) const noexcept { return m_CurrKeyboardKeyStatus[key]; }
            [[nodiscard]] bool isKeyDownOnce(uint32_t key) noexcept;
            [[nodiscard]] bool isKeyDownOnce(uint32_t key1, uint32_t key2) noexcept;
            [[nodiscard]] bool isKeyDownOnce(uint32_t key1, uint32_t key2, uint32_t key3) noexcept;
    };
}

#endif