#include <core/engine/utils/PrecompiledHeader.h>

#include <core/engine/events/EventModule.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

#include <SFML/Window.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

EventModule* eventModule = nullptr;

EventModule::EventModule(){
    eventModule = this;
}
EventModule::~EventModule(){
}
void EventModule::onEventMouseButtonPressed(unsigned int mouseButton){
    m_MouseModule.onButtonPressed(mouseButton);
}
void EventModule::onEventMouseButtonReleased(unsigned int mouseButton){
    m_MouseModule.onButtonReleased(mouseButton);
}
void EventModule::onPostUpdate(){
    m_KeyboardModule.onPostUpdate();
    m_MouseModule.onPostUpdate();
}
void EventModule::onClearEvents() {
    m_KeyboardModule.onClearEvents();
    m_MouseModule.onClearEvents();
}

#pragma region Keyboard
KeyboardKey::Key Engine::getPressedKey() {
    return eventModule->m_KeyboardModule.getCurrentPressedKey();
}
void EventModule::onEventKeyPressed(unsigned int key) {
    m_KeyboardModule.onKeyPressed(key);
}
void EventModule::onEventKeyReleased(unsigned int key) {
    m_KeyboardModule.onKeyReleased(key);
}
bool Engine::isKeyDown(KeyboardKey::Key key) {
    return eventModule->m_KeyboardModule.isKeyDown(key);
}
bool Engine::isKeyDownOnce() {
    return eventModule->m_KeyboardModule.isKeyDownOnce();
}
unsigned int Engine::getNumPressedKeys() {
    return eventModule->m_KeyboardModule.getNumPressedKeys();
}
bool Engine::isKeyDownOnce(KeyboardKey::Key key) {
    return eventModule->m_KeyboardModule.isKeyDownOnce(key);
}

bool Engine::isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second) {
    return eventModule->m_KeyboardModule.isKeyDownOnce(first, second);
}
bool Engine::isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second, KeyboardKey::Key third) {
    return eventModule->m_KeyboardModule.isKeyDownOnce(first, second, third);
}
bool Engine::isKeyUp(KeyboardKey::Key key) {
    return !Engine::isKeyDown(key);
}
#pragma endregion

#pragma region Mouse
MouseButton::Button Engine::getPressedButton() {
    return eventModule->m_MouseModule.getCurrentPressedButton();
}
unsigned int Engine::getNumPressedMouseButtons() {
    return eventModule->m_MouseModule.getNumPressedButtons();
}
bool Engine::isMouseButtonDown(MouseButton::Button button){
    return eventModule->m_MouseModule.isButtonDown(button);
}
bool Engine::isMouseButtonDownOnce(MouseButton::Button button){
    return eventModule->m_MouseModule.isButtonDownOnce(button);
}
const glm::vec2& Engine::getMouseDifference() {
    return Engine::getMouseDifference(Resources::getWindow());
}
const glm::vec2& Engine::getMousePositionPrevious() {
    return Engine::getMousePositionPrevious(Resources::getWindow());
}
const glm::vec2& Engine::getMousePosition() {
    return Engine::getMousePosition(Resources::getWindow());
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
    return Engine::getMouseWheelDelta(Resources::getWindow());
}
double Engine::getMouseWheelDelta(Window& window){
    return window.getMouseWheelDelta();
}
void Engine::setMousePosition(Window& window, float x, float y, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)), window.getSFMLHandle());
    window.updateMousePosition(x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, unsigned int x, unsigned int y, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x, y), window.getSFMLHandle());
    window.updateMousePosition(static_cast<float>(x), static_cast<float>(y), resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(pos.x), static_cast<int>(pos.y)), window.getSFMLHandle());
    window.updateMousePosition(pos.x, pos.y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x, pos.y), window.getSFMLHandle());
    window.updateMousePosition(static_cast<float>(pos.x), static_cast<float>(pos.y), resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(float x, float y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(unsigned int x, unsigned int y, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::vec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::uvec2& pos, bool resetDifference, bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}
#pragma endregion

