#include <serenity/input/Keyboard/KeyboardModule.h>
#include <SFML/Window/Keyboard.hpp>

using namespace Engine::priv;

void KeyboardModule::onKeyPressed(uint32_t key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_PreviousKeyboardKey = m_CurrentKeyboardKey;
    m_CurrentKeyboardKey = key;
    if (!m_KeyboardKeyStatus[key]) {
        m_KeyboardKeyStatus[key] = true;
        ++m_NumPressedKeys;
    }
}
void KeyboardModule::onKeyReleased(uint32_t key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
    m_CurrentKeyboardKey = (uint32_t)KeyboardKey::Unknown;
    if (m_KeyboardKeyStatus[key]) {
        m_KeyboardKeyStatus[key] = false;
        if (m_NumPressedKeys > 0) {
            --m_NumPressedKeys;
        }
    }
}
void KeyboardModule::postUpdate() noexcept {
    m_CurrentKeyboardKey = (uint32_t)KeyboardKey::Unknown;
    m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
}
void KeyboardModule::onClearEvents() noexcept {
    m_KeyboardKeyStatus.fill(false);
    m_CurrentKeyboardKey = (uint32_t)KeyboardKey::Unknown;
    m_PreviousKeyboardKey = (uint32_t)KeyboardKey::Unknown;
    m_NumPressedKeys = 0;
}
bool KeyboardModule::isKeyDownOnce(uint32_t key) noexcept {
    return isKeyDown(key) && m_CurrentKeyboardKey == key && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}
bool KeyboardModule::isKeyDownOnce(uint32_t key1, uint32_t key2) noexcept {
    const bool resFirst = isKeyDown(key1);
    const bool resSecond = isKeyDown(key2);
    return resFirst && resSecond && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}
bool KeyboardModule::isKeyDownOnce(uint32_t key1, uint32_t key2, uint32_t key3) noexcept {
    const bool resFirst = isKeyDown(key1);
    const bool resSecond = isKeyDown(key2);
    const bool resThird = isKeyDown(key3);
    return resFirst && resSecond && resThird && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}