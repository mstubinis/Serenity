#include <serenity/input/Keyboard/KeyboardModule.h>
#include <SFML/Window/Keyboard.hpp>

void Engine::priv::KeyboardModule::onWindowLostFocus() noexcept {
    std::fill(std::begin(m_CurrKeyboardKeyStatus), std::end(m_CurrKeyboardKeyStatus), false);
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onWindowGainedFocus() noexcept {
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onKeyPressed(KeyboardKey key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = true;
    ++m_NumPressedKeys;
}
void Engine::priv::KeyboardModule::onKeyReleased(KeyboardKey key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = false;
    m_NumPressedKeys = std::max(m_NumPressedKeys - 1, 0);
}
void Engine::priv::KeyboardModule::postUpdate() noexcept {
    m_PrevKeyboardKeyStatus = m_CurrKeyboardKeyStatus;
    m_CurrKeyboardKeyStatus[KeyboardKey::Unknown] = true;
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key) noexcept {
    return isKeyDown(key) && !m_PrevKeyboardKeyStatus[key];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && !m_PrevKeyboardKeyStatus[key1];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && isKeyDown(key3) && !m_PrevKeyboardKeyStatus[key1];
}