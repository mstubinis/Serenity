#include <serenity/events/EventModule.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/window/Window.h>

#include <SFML/Window.hpp>

Engine::view_ptr<Engine::priv::EventModule> EVENT_MODULE = nullptr;

Engine::priv::EventModule::EventModule() {
    EVENT_MODULE = this;
}
void Engine::priv::EventModule::onEventMouseButtonPressed(uint32_t mouseButton){
    m_MouseModule.onButtonPressed(mouseButton);
}
void Engine::priv::EventModule::onEventMouseButtonReleased(uint32_t mouseButton){
    m_MouseModule.onButtonReleased(mouseButton);
}
void Engine::priv::EventModule::onWindowLostFocus() {
    m_KeyboardModule.onWindowLostFocus();
    m_MouseModule.onWindowLostFocus();
    m_JoystickModule.onWindowLostFocus();
}
void Engine::priv::EventModule::onWindowGainedFocus() {
    m_KeyboardModule.onWindowGainedFocus();
    m_MouseModule.onWindowGainedFocus();
    m_JoystickModule.onWindowGainedFocus();
}
void Engine::priv::EventModule::postUpdate(){
    m_EventDispatcher.postUpdate();
}
void Engine::priv::EventModule::onEventKeyPressed(uint32_t key) {
    m_KeyboardModule.onKeyPressed(key);
}
void Engine::priv::EventModule::onEventKeyReleased(uint32_t key) {
    m_KeyboardModule.onKeyReleased(key);
}

#pragma region Keyboard
uint32_t Engine::getNumPressedKeys() {
    return EVENT_MODULE->m_KeyboardModule.getNumPressedKeys();
}
bool Engine::isKeyDown(KeyboardKey::Key key) {
    return EVENT_MODULE->m_KeyboardModule.isKeyDown(key);
}
bool Engine::isKeyDownOnce(KeyboardKey::Key key) {
    return EVENT_MODULE->m_KeyboardModule.isKeyDownOnce(key);
}
bool Engine::isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second) {
    return EVENT_MODULE->m_KeyboardModule.isKeyDownOnce(first, second);
}
bool Engine::isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second, KeyboardKey::Key third) {
    return EVENT_MODULE->m_KeyboardModule.isKeyDownOnce(first, second, third);
}
bool Engine::isKeyUp(KeyboardKey::Key key) {
    return !Engine::isKeyDown(key);
}
bool Engine::isKeyDown(uint32_t key) {
    return Engine::isKeyDown((KeyboardKey::Key)key);
}
bool Engine::isKeyDownOnce(uint32_t key) {
    return Engine::isKeyDownOnce((KeyboardKey::Key)key);
}
bool Engine::isKeyDownOnce(uint32_t first, uint32_t second) {
    return Engine::isKeyDownOnce((KeyboardKey::Key)first, (KeyboardKey::Key)second);
}
bool Engine::isKeyDownOnce(uint32_t first, uint32_t second, uint32_t third) {
    return Engine::isKeyDownOnce((KeyboardKey::Key)first, (KeyboardKey::Key)second, (KeyboardKey::Key)third);
}
bool Engine::isKeyUp(uint32_t key) {
    return Engine::isKeyUp((KeyboardKey::Key)key);
}

#pragma endregion

#pragma region Mouse
uint32_t Engine::getNumPressedMouseButtons() {
    return EVENT_MODULE->m_MouseModule.getNumPressedButtons();
}
const glm::vec2& Engine::getMouseDifference() {
    return Engine::getMouseDifference(Engine::Resources::getWindow());
}
const glm::vec2& Engine::getMousePositionPrevious() {
    return Engine::getMousePositionPrevious(Engine::Resources::getWindow());
}
const glm::vec2& Engine::getMousePosition() {
    return Engine::getMousePosition(Engine::Resources::getWindow());
}
const glm::vec2& Engine::getMouseDifference(Window& window){
    return window.getMousePositionDifference();
}
const glm::vec2& Engine::getMousePositionPrevious(Window& window){
    return window.getMousePositionPrevious();
}
const glm::vec2& Engine::getMousePosition(Window& window){
    return window.getMousePosition();
}
double Engine::getMouseWheelDelta() {
    return Engine::getMouseWheelDelta(Engine::Resources::getWindow());
}
double Engine::getMouseWheelDelta(Window& window){
    return window.getMouseWheelDelta();
}
void Engine::setMousePosition(Window& window, float x, float y, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i((int)x, (int)y), window.getSFMLHandle());
    window.updateMousePosition(x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, uint32_t x, uint32_t y, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x, y), window.getSFMLHandle());
    window.updateMousePosition((float)x, (float)y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i((int)pos.x, (int)pos.y), window.getSFMLHandle());
    window.updateMousePosition(pos.x, pos.y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x, pos.y), window.getSFMLHandle());
    window.updateMousePosition((float)pos.x, (float)pos.y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(float x, float y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(uint32_t x, uint32_t y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Engine::Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}
bool Engine::isMouseButtonDown(MouseButton::Button button) {
    return EVENT_MODULE->m_MouseModule.isButtonDown(button);
}
bool Engine::isMouseButtonDownOnce(MouseButton::Button button) {
    return EVENT_MODULE->m_MouseModule.isButtonDownOnce(button);
}
bool Engine::isMouseButtonDown(uint32_t button) {
    return Engine::isMouseButtonDown((MouseButton::Button)button);
}
bool Engine::isMouseButtonDownOnce(uint32_t button) {
    return Engine::isMouseButtonDownOnce((MouseButton::Button)button);
}

#pragma endregion
