#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <core/engine/input/Keyboard/KeyboardKeys.h>
#include <array>

namespace Engine::priv {
    class KeyboardModule {
        private:
            std::array<bool, KeyboardKey::_TOTAL>  m_KeyboardKeyStatus;
            unsigned int                           m_CurrentKeyboardKey  = (unsigned int)KeyboardKey::Unknown;
            unsigned int                           m_PreviousKeyboardKey = (unsigned int)KeyboardKey::Unknown;
            unsigned int                           m_NumPressedKeys      = 0U;
        public:
            KeyboardModule() {
                m_KeyboardKeyStatus.fill(false);
            }
            virtual ~KeyboardModule() {}

            constexpr void onKeyPressed(unsigned int key) {
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
            constexpr void onKeyReleased(unsigned int key) {
                if (key == KeyboardKey::Unknown) {
                    return;
                }
                m_PreviousKeyboardKey = (unsigned int)KeyboardKey::Unknown;
                m_CurrentKeyboardKey  = (unsigned int)KeyboardKey::Unknown;
                if (m_KeyboardKeyStatus[key] == true) {
                    m_KeyboardKeyStatus[key] = false;
                    if (m_NumPressedKeys > 0) {
                        --m_NumPressedKeys;
                    }
                }
            }
            constexpr void onPostUpdate() {
                m_CurrentKeyboardKey  = (unsigned int)KeyboardKey::Unknown;
                m_PreviousKeyboardKey = (unsigned int)KeyboardKey::Unknown;
            }
            void onClearEvents() {
                m_KeyboardKeyStatus.fill(false);
                m_CurrentKeyboardKey  = (unsigned int)KeyboardKey::Unknown;
                m_PreviousKeyboardKey = (unsigned int)KeyboardKey::Unknown;
                m_NumPressedKeys      = 0U;
            }
            constexpr KeyboardKey::Key getCurrentPressedKey() const {
                return (KeyboardKey::Key)m_CurrentKeyboardKey;
            }
            constexpr unsigned int getNumPressedKeys() const {
                return m_NumPressedKeys;
            }
            constexpr bool isKeyDown(unsigned int key) const {
                return (m_KeyboardKeyStatus[key] == true);
            }
            constexpr bool isKeyDownOnce() const {
                return m_CurrentKeyboardKey != m_PreviousKeyboardKey;
            }
            constexpr bool isKeyDownOnce(unsigned int key) {
                const bool res = isKeyDown(key);
                return res && m_CurrentKeyboardKey == key && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
            constexpr bool isKeyDownOnce(unsigned int key1, unsigned int key2) {
                const bool resFirst  = isKeyDown(key1);
                const bool resSecond = isKeyDown(key2);
                return resFirst && resSecond && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
            constexpr bool isKeyDownOnce(unsigned int key1, unsigned int key2, unsigned int key3) {
                const bool resFirst  = isKeyDown(key1);
                const bool resSecond = isKeyDown(key2);
                const bool resThird  = isKeyDown(key3);
                return resFirst && resSecond && resThird && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
            }
    };
}

#endif