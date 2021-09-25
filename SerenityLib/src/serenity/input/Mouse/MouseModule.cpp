#include <serenity/input/Mouse/MouseModule.h>

void Engine::priv::MouseModule::onWindowLostFocus() noexcept {
    std::fill(std::begin(m_CurrMouseStatus), std::end(m_CurrMouseStatus), false);
    m_NumPressedMouseButtons = 0;
}
void Engine::priv::MouseModule::onWindowGainedFocus() noexcept {
    m_NumPressedMouseButtons = 0;
}
void Engine::priv::MouseModule::onButtonPressed(MouseButton button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_CurrMouseStatus[button] = true;
    ++m_NumPressedMouseButtons;
}
void Engine::priv::MouseModule::onButtonReleased(MouseButton button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_CurrMouseStatus[button] = false;
    m_NumPressedMouseButtons = std::max(m_NumPressedMouseButtons - 1, 0);
}
void Engine::priv::MouseModule::postUpdate() noexcept {
    m_PrevMouseStatus = m_CurrMouseStatus;
    m_CurrMouseStatus[MouseButton::Unknown] = true;
}
bool Engine::priv::MouseModule::isButtonDownOnce(MouseButton button) noexcept {
    return isButtonDown(button) && m_PrevMouseStatus[button] == false;
}