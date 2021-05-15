#include <serenity/input/Mouse/MouseModule.h>

void Engine::priv::MouseModule::onWindowLostFocus() noexcept {
    m_CurrMouseStatus.fill(false);
    m_NumPressedMouseButtons = 0;
}
void Engine::priv::MouseModule::onWindowGainedFocus() noexcept {
    m_NumPressedMouseButtons = 0;
}
void Engine::priv::MouseModule::onButtonPressed(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_CurrMouseStatus[button] = true;
    ++m_NumPressedMouseButtons;
}
void Engine::priv::MouseModule::onButtonReleased(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_CurrMouseStatus[button] = false;
    m_NumPressedMouseButtons = std::max(m_NumPressedMouseButtons - 1U, 0U);
}
void Engine::priv::MouseModule::update() noexcept {
    m_PrevMouseStatus = m_CurrMouseStatus;
    m_CurrMouseStatus[(uint32_t)MouseButton::Unknown] = true;
}
bool Engine::priv::MouseModule::isButtonDownOnce(uint32_t button) noexcept {
    return isButtonDown(button) && m_PrevMouseStatus[button] == false;
}