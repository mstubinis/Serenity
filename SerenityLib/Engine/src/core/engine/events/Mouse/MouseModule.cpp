#include <core/engine/events/Mouse/MouseModule.h>

using namespace Engine::priv;
using namespace std;

MouseModule::MouseModule() {
    m_MouseStatus.fill(false);
}
MouseModule::~MouseModule() {

}

void MouseModule::onButtonPressed(unsigned int button) {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_PreviousMouseButton = m_CurrentMouseButton;
    m_CurrentMouseButton  = button;

    if (!m_MouseStatus[button]) {
        m_MouseStatus[button] = true;
    }
}
void MouseModule::onButtonReleased(unsigned int button) {
    if (button == MouseButton::Unknown) {
        return;
    }
    m_PreviousMouseButton = static_cast<unsigned int>(MouseButton::Unknown);
    m_CurrentMouseButton  = static_cast<unsigned int>(MouseButton::Unknown);

    if (m_MouseStatus[button]) {
        m_MouseStatus[button] = false;
    }
}
void MouseModule::onPostUpdate() {
    m_PreviousMouseButton    = static_cast<unsigned int>(MouseButton::Unknown);
    m_CurrentMouseButton     = static_cast<unsigned int>(MouseButton::Unknown);
    m_NumPressedMouseButtons = 0U;
}
void MouseModule::onClearEvents() {
    m_MouseStatus.fill(false);
    m_PreviousMouseButton    = static_cast<unsigned int>(MouseButton::Unknown);
    m_CurrentMouseButton     = static_cast<unsigned int>(MouseButton::Unknown);
    m_NumPressedMouseButtons = 0U;
}
MouseButton::Button MouseModule::getCurrentPressedButton() const {
    return static_cast<MouseButton::Button>(m_CurrentMouseButton);
}
unsigned int MouseModule::getNumPressedButtons() const {
    return m_NumPressedMouseButtons;
}
bool MouseModule::isButtonDown(unsigned int button) {
    return (m_MouseStatus[button] == true);
}
bool MouseModule::isButtonDownOnce(unsigned int button) {
    bool res = isButtonDown(button);
    return res && m_CurrentMouseButton == button && m_CurrentMouseButton != m_PreviousMouseButton;
}