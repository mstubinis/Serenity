#include <serenity/input/InputModule.h>

#include <serenity/system/Engine.h>
#include <serenity/system/window/Window.h>
#include <serenity/types/ViewPointer.h>

namespace {
    Engine::view_ptr<Engine::priv::InputModule> INPUT_MODULE = nullptr;
}



Engine::priv::InputModule::InputModule() {
    INPUT_MODULE = this;
}
void Engine::priv::InputModule::onEventMouseButtonPressed(uint32_t mouseButton) {
    m_MouseModule.onButtonPressed(mouseButton);
}
void Engine::priv::InputModule::onEventMouseButtonReleased(uint32_t mouseButton) {
    m_MouseModule.onButtonReleased(mouseButton);
}
void Engine::priv::InputModule::onWindowLostFocus() {
    m_KeyboardModule.onWindowLostFocus();
    m_MouseModule.onWindowLostFocus();
    m_JoystickModule.onWindowLostFocus();
}
void Engine::priv::InputModule::onWindowGainedFocus() {
    m_KeyboardModule.onWindowGainedFocus();
    m_MouseModule.onWindowGainedFocus();
    m_JoystickModule.onWindowGainedFocus();
}
void Engine::priv::InputModule::onEventKeyPressed(uint32_t key) {
    m_KeyboardModule.onKeyPressed(key);
}
void Engine::priv::InputModule::onEventKeyReleased(uint32_t key) {
    m_KeyboardModule.onKeyReleased(key);
}

#pragma region Keyboard
uint32_t Engine::input::getNumPressedKeys() {
    return INPUT_MODULE->getKeyboardModule().getNumPressedKeys();
}
bool Engine::input::isKeyDown(KeyboardKey key) {
    return INPUT_MODULE->getKeyboardModule().isKeyDown(key);
}
bool Engine::input::isKeyDownOnce(KeyboardKey key) {
    return INPUT_MODULE->getKeyboardModule().isKeyDownOnce(key);
}
bool Engine::input::isKeyDownOnce(KeyboardKey first, KeyboardKey second) {
    return INPUT_MODULE->getKeyboardModule().isKeyDownOnce(first, second);
}
bool Engine::input::isKeyDownOnce(KeyboardKey first, KeyboardKey second, KeyboardKey third) {
    return INPUT_MODULE->getKeyboardModule().isKeyDownOnce(first, second, third);
}
bool Engine::input::isKeyUp(KeyboardKey key) {
    return !Engine::isKeyDown(key);
}
bool Engine::input::isCapsLockOn() {
    return INPUT_MODULE->getKeyboardModule().isCapsLockOn();
}
bool Engine::input::isNumLockOn() {
    return INPUT_MODULE->getKeyboardModule().isNumLockOn();
}
bool Engine::input::isScrollLockOn() {
    return INPUT_MODULE->getKeyboardModule().isScrollLockOn();
}

#pragma endregion

#pragma region Mouse
uint32_t Engine::input::getNumPressedMouseButtons() {
    return INPUT_MODULE->getMouseModule().getNumPressedButtons();
}
const glm::vec2& Engine::input::getMouseDifference() {
    return Engine::getMouseDifference(Engine::getWindow());
}
const glm::vec2& Engine::input::getMousePositionPrevious() {
    return Engine::getMousePositionPrevious(Engine::getWindow());
}
const glm::vec2& Engine::input::getMousePosition() {
    return Engine::getMousePosition(Engine::getWindow());
}
const glm::vec2& Engine::input::getMouseDifference(Window& window) {
    return window.getMousePositionDifference();
}
const glm::vec2& Engine::input::getMousePositionPrevious(Window& window) {
    return window.getMousePositionPrevious();
}
const glm::vec2& Engine::input::getMousePosition(Window& window) {
    return window.getMousePosition();
}
double Engine::input::getMouseWheelDelta() {
    return Engine::getMouseWheelDelta(Engine::getWindow());
}
double Engine::input::getMouseWheelDelta(Window& window) {
    return window.getMouseWheelDelta();
}
void Engine::input::setMousePosition(Window& window, float x, float y, bool resetDifference, bool resetPreviousPosition) {
    sf::Mouse::setPosition(sf::Vector2i((int)x, (int)y), window.getSFMLHandle());
    window.updateMousePosition(x, y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(Window& window, uint32_t x, uint32_t y, bool resetDifference, bool resetPreviousPosition) {
    sf::Mouse::setPosition(sf::Vector2i(x, y), window.getSFMLHandle());
    window.updateMousePosition((float)x, (float)y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(Window& window, const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition) {
    sf::Mouse::setPosition(sf::Vector2i((int)pos.x, (int)pos.y), window.getSFMLHandle());
    window.updateMousePosition(pos.x, pos.y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(Window& window, const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition) {
    sf::Mouse::setPosition(sf::Vector2i(pos.x, pos.y), window.getSFMLHandle());
    window.updateMousePosition((float)pos.x, (float)pos.y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(float x, float y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(uint32_t x, uint32_t y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::getWindow(), pos, resetDifference, resetPreviousPosition);
}
void Engine::input::setMousePosition(const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::getWindow(), pos, resetDifference, resetPreviousPosition);
}
bool Engine::input::isMouseButtonDown(MouseButton button) {
    return INPUT_MODULE->getMouseModule().isButtonDown(button);
}
bool Engine::input::isMouseButtonDownOnce(MouseButton button) {
    return INPUT_MODULE->getMouseModule().isButtonDownOnce(button);
}

#pragma endregion

#pragma region Joystick

#pragma endregion