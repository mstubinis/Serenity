#include <serenity/core/engine/events/EventModule.h>
#include <serenity/core/engine/resources/Engine_Resources.h>
#include <serenity/core/engine/system/window/Window.h>

#include <SFML/Window.hpp>

using namespace Engine;
using namespace Engine::priv;

Engine::view_ptr<EventModule> EVENT_MODULE = nullptr;

EventModule::EventModule() {
    EVENT_MODULE = this;
}
void EventModule::onEventMouseButtonPressed(uint32_t mouseButton){
    m_MouseModule.onButtonPressed(mouseButton);
}
void EventModule::onEventMouseButtonReleased(uint32_t mouseButton){
    m_MouseModule.onButtonReleased(mouseButton);
}
void EventModule::postUpdate(){
    m_KeyboardModule.postUpdate();
    m_MouseModule.postUpdate();
    m_EventDispatcher.postUpdate();
}
void EventModule::onClearEvents() {
    m_KeyboardModule.onClearEvents();
    m_MouseModule.onClearEvents();
}

#pragma region Keyboard
KeyboardKey::Key Engine::getPressedKey() {
    return EVENT_MODULE->m_KeyboardModule.getCurrentPressedKey();
}
void EventModule::onEventKeyPressed(uint32_t key) {
    m_KeyboardModule.onKeyPressed(key);
}
void EventModule::onEventKeyReleased(uint32_t key) {
    m_KeyboardModule.onKeyReleased(key);
}
bool Engine::isKeyDown(KeyboardKey::Key key) {
    return EVENT_MODULE->m_KeyboardModule.isKeyDown(key);
}
bool Engine::isKeyDownOnce() {
    return EVENT_MODULE->m_KeyboardModule.isKeyDownOnce();
}
uint32_t Engine::getNumPressedKeys() {
    return EVENT_MODULE->m_KeyboardModule.getNumPressedKeys();
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
#pragma endregion

#pragma region Mouse
MouseButton::Button Engine::getPressedButton() {
    return EVENT_MODULE->m_MouseModule.getCurrentPressedButton();
}
uint32_t Engine::getNumPressedMouseButtons() {
    return EVENT_MODULE->m_MouseModule.getNumPressedButtons();
}
bool Engine::isMouseButtonDown(MouseButton::Button button){
    return EVENT_MODULE->m_MouseModule.isButtonDown(button);
}
bool Engine::isMouseButtonDownOnce(MouseButton::Button button){
    return EVENT_MODULE->m_MouseModule.isButtonDownOnce(button);
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
#pragma endregion
