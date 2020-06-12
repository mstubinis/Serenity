#include <core/engine/events/Keyboard/KeyboardModule.h>
#include <SFML/Window/Keyboard.hpp>

using namespace Engine::priv;
using namespace std;

#include <iostream>

KeyboardModule::KeyboardModule() {
    m_KeyboardKeyStatus.fill(false);
}
KeyboardModule::~KeyboardModule() {

}
void KeyboardModule::onKeyPressed(unsigned int key) {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_PreviousKeyboardKey = m_CurrentKeyboardKey;
    m_CurrentKeyboardKey  = key;

    if (m_KeyboardKeyStatus[key] == false) {
        m_KeyboardKeyStatus[key] = true;
        ++m_NumPressedKeys;
    }
}
void KeyboardModule::onKeyReleased(unsigned int key) {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_PreviousKeyboardKey = static_cast<unsigned int>(KeyboardKey::Unknown);
    m_CurrentKeyboardKey  = static_cast<unsigned int>(KeyboardKey::Unknown);

    if (m_KeyboardKeyStatus[key] == true) {
        m_KeyboardKeyStatus[key] = false;
        if (m_NumPressedKeys > 0) {
            --m_NumPressedKeys;
        }
    }
}
void KeyboardModule::onPostUpdate() {
    m_CurrentKeyboardKey  = static_cast<unsigned int>(KeyboardKey::Unknown);
    m_PreviousKeyboardKey = static_cast<unsigned int>(KeyboardKey::Unknown);
}
void KeyboardModule::onClearEvents() {
    m_KeyboardKeyStatus.fill(false);
    m_CurrentKeyboardKey  = static_cast<unsigned int>(KeyboardKey::Unknown);
    m_PreviousKeyboardKey = static_cast<unsigned int>(KeyboardKey::Unknown);
    m_NumPressedKeys      = 0U;
}
KeyboardKey::Key KeyboardModule::getCurrentPressedKey() const {
    return static_cast<KeyboardKey::Key>(m_CurrentKeyboardKey);
}
bool KeyboardModule::isKeyDown(unsigned int key) const {
    return (m_KeyboardKeyStatus[key] == true);
}
bool KeyboardModule::isKeyDownOnce() const {
    return m_CurrentKeyboardKey != m_PreviousKeyboardKey;
}

bool KeyboardModule::isKeyDownOnce(unsigned int key) {
    bool res = isKeyDown(key);
    return res && m_CurrentKeyboardKey == key && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}
bool KeyboardModule::isKeyDownOnce(unsigned int key1, unsigned int key2) {
    bool resFirst  = isKeyDown(key1);
    bool resSecond = isKeyDown(key2);
    return resFirst && resSecond && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}
bool KeyboardModule::isKeyDownOnce(unsigned int key1, unsigned int key2, unsigned int key3) {
    bool resFirst  = isKeyDown(key1);
    bool resSecond = isKeyDown(key2);
    bool resThird  = isKeyDown(key3);
    return resFirst && resSecond && resThird && m_CurrentKeyboardKey == key1 && (m_CurrentKeyboardKey != m_PreviousKeyboardKey);
}
unsigned int KeyboardModule::getNumPressedKeys() const {
    return m_NumPressedKeys;
}