#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/system/TypeDefs.h>
#include <bitset>

namespace Engine::priv {
    class KeyboardModule final {
        public:
            //using KeyStatusArray = std::vector<bool>;
            using KeyStatusArray = std::bitset<KeyboardKey::_TOTAL>;
        private:
            KeyStatusArray  m_CurrKeyboardKeyStatus;
            KeyStatusArray  m_PrevKeyboardKeyStatus;
            int32_t         m_NumPressedKeys        = 0;
        public:
            KeyboardModule();
            void onWindowLostFocus() noexcept;
            void onWindowGainedFocus() noexcept;
            void onKeyPressed(KeyboardKey) noexcept;
            void onKeyReleased(KeyboardKey) noexcept;
            void postUpdate() noexcept;

            [[nodiscard]] inline constexpr int32_t getNumPressedKeys() const noexcept { return m_NumPressedKeys; }
            [[nodiscard]] inline constexpr bool isKeyDown(KeyboardKey key) const noexcept { return m_CurrKeyboardKeyStatus[key]; }
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey) const noexcept;
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) const noexcept;
            [[nodiscard]] bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) const noexcept;
            [[nodiscard]] bool isCapsLockOn() const noexcept;
            [[nodiscard]] bool isScrollLockOn() const noexcept;
            [[nodiscard]] bool isNumLockOn() const noexcept;
    };
}

#endif