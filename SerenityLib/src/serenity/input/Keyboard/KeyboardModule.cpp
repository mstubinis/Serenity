#include <serenity/input/Keyboard/KeyboardModule.h>
#include <SFML/Window/Keyboard.hpp>

void Engine::priv::KeyboardModule::onWindowLostFocus() noexcept {
    m_CurrKeyboardKeyStatus.fill(false);
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onWindowGainedFocus() noexcept {
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onKeyPressed(uint32_t key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = true;
    ++m_NumPressedKeys;
}
void Engine::priv::KeyboardModule::onKeyReleased(uint32_t key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = false;
    m_NumPressedKeys = std::max(m_NumPressedKeys - 1U, 0U);
}
void Engine::priv::KeyboardModule::update() noexcept {
    m_PrevKeyboardKeyStatus = m_CurrKeyboardKeyStatus;
    m_CurrKeyboardKeyStatus[(uint32_t)KeyboardKey::Unknown] = true;
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(uint32_t key) noexcept {
    return isKeyDown(key) && !m_PrevKeyboardKeyStatus[key];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(uint32_t key1, uint32_t key2) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && !m_PrevKeyboardKeyStatus[key1];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(uint32_t key1, uint32_t key2, uint32_t key3) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && isKeyDown(key3) && !m_PrevKeyboardKeyStatus[key1];
}