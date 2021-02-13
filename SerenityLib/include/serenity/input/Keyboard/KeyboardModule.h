#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/system/TypeDefs.h>
#include <array>

namespace Engine::priv {
    class KeyboardModule {
        private:
            std::array<bool, KeyboardKey::_TOTAL>  m_KeyboardKeyStatus;
            uint32_t                               m_CurrentKeyboardKey  = (uint32_t)KeyboardKey::Unknown;
            uint32_t                               m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
            uint32_t                               m_NumPressedKeys      = 0U;
        public:
            KeyboardModule() {
                m_KeyboardKeyStatus.fill(false);
            }
            virtual ~KeyboardModule() = default;

            constexpr void onKeyPressed(uint32_t key) noexcept {
                if (key == KeyboardKey::Unknown) {
                    return;
                }
                m_PreviousKeyboardKey = m_CurrentKeyboardKey;
                m_CurrentKeyboardKey = key;
                if (m_KeyboardKeyStatus[key] == false) {
                    m_KeyboardKeyStatus[key] = true;
                    ++m_NumPressedKeys;
                }
            }
            constexpr void onKeyReleased(uint32_t key) noexcept {
                if (key == KeyboardKey::Unknown) {
                    return;
                }
                m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
                m_CurrentKeyboardKey  = (uint32_t)KeyboardKey::Unknown;
                if (m_KeyboardKeyStatus[key] == true) {
                    m_KeyboardKeyStatus[key] = false;
                    if (m_NumPressedKeys > 0) {
                        --m_NumPressedKeys;
                    }
                }
            }
            constexpr void postUpdate() noexcept {
                m_CurrentKeyboardKey  = (uint32_t)KeyboardKey::Unknown;
                m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
            }
            void onClearEvents() noexcept {
                m_KeyboardKeyStatus.fill(false);
                m_CurrentKeyboardKey  = (uint32_t)KeyboardKey::Unknown;
                m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
                m_NumPressedKeys      = 0U;
            }
            [[nodiscard]] inline constexpr KeyboardKey::Key getCurrentPressedKey() const noexcept {
                return (KeyboardKey::Key)m_CurrentKeyboardKey;
            }
            [[nodiscard]] inline constexpr uint32_t getNumPressedKeys() const noexcept {
                return m_NumPressedKeys;
            }
            [[nodiscard]] inline constexpr bool isKeyDown(uint32_t key) const noexcept {
                return (m_KeyboardKeyStatus[key] == true);
            }
            [[nodiscard]] inline constexpr bool isKeyDownOnce() const noexcept {
                return m_CurrentKeyboardKey != m_PreviousKeyboardKey;
            }
            [[nodiscard]] constexpr bool isKeyDownOnce(uint32_t key) noexcept {
                const bool res = isKeyDown(key);
                return res && m_CurrentKeyboardKey == key && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
            [[nodiscard]] constexpr bool isKeyDownOnce(uint32_t key1, uint32_t key2) noexcept {
                const bool resFirst  = isKeyDown(key1);
                const bool resSecond = isKeyDown(key2);
                return resFirst && resSecond && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
            [[nodiscard]] constexpr bool isKeyDownOnce(uint32_t key1, uint32_t key2, uint32_t key3) noexcept {
                const bool resFirst  = isKeyDown(key1);
                const bool resSecond = isKeyDown(key2);
                const bool resThird  = isKeyDown(key3);
                return resFirst && resSecond && resThird && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
    };
}

#endif