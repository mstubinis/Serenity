#include <serenity/input/Mouse/MouseModule.h>

using namespace Engine::priv;

void MouseModule::onButtonPressed(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_PreviousMouseButton = m_CurrentMouseButton;
    m_CurrentMouseButton  = button;
    if (!m_MouseStatus[button]) {
        m_MouseStatus[button] = true;
    }
}
void MouseModule::onButtonReleased(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_PreviousMouseButton = (uint32_t)MouseButton::Unknown;
    m_CurrentMouseButton  = (uint32_t)MouseButton::Unknown;
    if (m_MouseStatus[button]) {
        m_MouseStatus[button] = false;
    }
}
void MouseModule::postUpdate() noexcept {
    m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
    m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
    m_NumPressedMouseButtons = 0;
}
void MouseModule::onClearEvents() noexcept {
    m_MouseStatus.fill(false);
    m_PreviousMouseButton    = (uint32_t)MouseButton::Unknown;
    m_CurrentMouseButton     = (uint32_t)MouseButton::Unknown;
    m_NumPressedMouseButtons = 0;
}
bool MouseModule::isButtonDownOnce(uint32_t button) noexcept {
    return isButtonDown(button) && m_CurrentMouseButton == button && m_CurrentMouseButton != m_PreviousMouseButton;
}