#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class KeyboardModule final {
        using KeyStatusArray = std::vector<bool>;
        private:
            KeyStatusArray  m_CurrKeyboardKeyStatus = KeyStatusArray(KeyboardKey::_TOTAL, false);
            KeyStatusArray  m_PrevKeyboardKeyStatus = KeyStatusArray(KeyboardKey::_TOTAL, false);
            int32_t         m_NumPressedKeys        = 0;
        public:
            void onWindowLostFocus() noexcept;
            void onWindowGainedFocus() noexcept;
            void onKeyPressed(uint32_t key) noexcept;
            void onKeyReleased(uint32_t key) noexcept;
            void update() noexcept;
            [[nodiscard]] inline constexpr int32_t getNumPressedKeys() const noexcept { return m_NumPressedKeys; }
            [[nodiscard]] inline constexpr bool isKeyDown(uint32_t key) const noexcept { return m_CurrKeyboardKeyStatus[key]; }
            [[nodiscard]] bool isKeyDownOnce(uint32_t key) noexcept;
            [[nodiscard]] bool isKeyDownOnce(uint32_t key1, uint32_t key2) noexcept;
            [[nodiscard]] bool isKeyDownOnce(uint32_t key1, uint32_t key2, uint32_t key3) noexcept;
    };
}

#endif