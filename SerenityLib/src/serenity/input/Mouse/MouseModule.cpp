#include <serenity/input/Mouse/MouseModule.h>

using namespace Engine::priv;

void MouseModule::onButtonPressed(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    if (!m_CurrMouseStatus[button]) {
        m_CurrMouseStatus[button] = true;
    }
}
void MouseModule::onButtonReleased(uint32_t button) noexcept {
    if (button == MouseButton::Unknown) {
        return;
    }
    if (m_CurrMouseStatus[button]) {
        m_CurrMouseStatus[button] = false;
    }
}
void MouseModule::update() noexcept {
    m_PrevMouseStatus = m_CurrMouseStatus;
    m_CurrMouseStatus[(uint32_t)MouseButton::Unknown] = true;
}
void MouseModule::onClearEvents() noexcept {
    m_NumPressedMouseButtons = 0;
}
bool MouseModule::isButtonDownOnce(uint32_t button) noexcept {
    return isButtonDown(button) && m_CurrMouseStatus[button] != m_PrevMouseStatus[button];
}