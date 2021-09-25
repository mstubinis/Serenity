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
            void onKeyPressed(KeyboardKey key) noexcept;
            void onKeyReleased(KeyboardKey key) noexcept;
            void postUpdate() noexcept;
            [[nodiscard]] inline constexpr int32_t getNumPressedKeys() const noexcept { return m_NumPressedKeys; }
            [[nodiscard]] inline constexpr bool isKeyDown(KeyboardKey key) const noexcept { return m_CurrKeyboardKeyStatus[key]; }
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey key) noexcept;
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) noexcept;
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) noexcept;
    };
}

#endif