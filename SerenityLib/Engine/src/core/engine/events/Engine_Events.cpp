#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

#include <SFML/Window.hpp>
#include <unordered_map>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

EventManager* eventManager = nullptr;

EventManager::EventManager(){
    m_CurrentKeyboardKey = m_PreviousKeyboardKey = KeyboardKey::Unknown;
    m_CurrentMouseButton = m_PreviousMouseButton = MouseButton::Unknown;

    eventManager = this;
}
EventManager::~EventManager(){
    cleanup();
}
void EventManager::cleanup() {
    m_KeyboardKeyStatus.clear();
    m_MouseStatus.clear();
}

void EventManager::onEventKeyPressed(const unsigned int key){
    m_PreviousKeyboardKey = m_CurrentKeyboardKey;
    m_CurrentKeyboardKey  = key;

    if (!m_KeyboardKeyStatus.count(key)) {
        m_KeyboardKeyStatus.insert(key);
    }
}
void EventManager::onEventKeyReleased(const unsigned int key){
    m_PreviousKeyboardKey = KeyboardKey::Unknown;
    m_CurrentKeyboardKey  = KeyboardKey::Unknown;

    if (m_KeyboardKeyStatus.count(key)) {
        m_KeyboardKeyStatus.erase(key);
    }
}
void EventManager::onEventMouseButtonPressed(const unsigned int mouseButton){
    m_PreviousMouseButton = m_CurrentMouseButton;
    m_CurrentMouseButton  = mouseButton;

    if (!m_MouseStatus.count(mouseButton)) {
        m_MouseStatus.insert(mouseButton);
    }
}
void EventManager::onEventMouseButtonReleased(const unsigned int mouseButton){
    m_PreviousMouseButton = MouseButton::Unknown;
    m_CurrentMouseButton  = MouseButton::Unknown;

    if (m_MouseStatus.count(mouseButton)) {
        m_MouseStatus.erase(mouseButton);
    }
}
void EventManager::onResetEvents(const float dt){
    m_PreviousKeyboardKey = KeyboardKey::Unknown;
    m_CurrentKeyboardKey  = KeyboardKey::Unknown;
    m_PreviousMouseButton = MouseButton::Unknown;
    m_CurrentMouseButton  = MouseButton::Unknown;
}
const KeyboardKey::Key Engine::getPressedKey() {
    return static_cast<KeyboardKey::Key>(eventManager->m_CurrentKeyboardKey);
}
const MouseButton::Button Engine::getPressedButton() {
    return static_cast<MouseButton::Button>(eventManager->m_CurrentMouseButton);
}
const bool Engine::isKeyDown(const KeyboardKey::Key key){
    return eventManager->m_KeyboardKeyStatus.count(key);
}
const bool Engine::isKeyDownOnce() {
    auto& mgr = *eventManager;
    return (mgr.m_CurrentKeyboardKey != mgr.m_PreviousKeyboardKey);
}



const unsigned int Engine::getNumPressedKeys() {
    auto& mgr = *eventManager;
    return static_cast<unsigned int>(mgr.m_KeyboardKeyStatus.size());
}
const unordered_set<unsigned int>& Engine::getPressedKeys() {
    auto& mgr = *eventManager;
    return mgr.m_KeyboardKeyStatus;
}
const unordered_set<unsigned int>& Engine::getPressedMouseButtons() {
    auto& mgr = *eventManager;
    return mgr.m_MouseStatus;
}
const bool Engine::isKeyDownOnce(const KeyboardKey::Key key){
    const bool res = Engine::isKeyDown(key);
    auto& mgr = *eventManager;
    return (res && mgr.m_CurrentKeyboardKey == key && (mgr.m_CurrentKeyboardKey != mgr.m_PreviousKeyboardKey));
}

const bool Engine::isKeyDownOnce(const KeyboardKey::Key first, const KeyboardKey::Key second) {
    const bool resFirst = Engine::isKeyDown(first);
    const bool resSecond = Engine::isKeyDown(second);
    auto& mgr = *eventManager;
    return ( resFirst && resSecond && mgr.m_CurrentKeyboardKey == first && (mgr.m_CurrentKeyboardKey != mgr.m_PreviousKeyboardKey));
}
const bool Engine::isKeyDownOnce(const KeyboardKey::Key first, const KeyboardKey::Key second, const KeyboardKey::Key third) {
    const bool resFirst = Engine::isKeyDown(first);
    const bool resSecond = Engine::isKeyDown(second);
    const bool resThird = Engine::isKeyDown(third);
    auto& mgr = *eventManager;
    return (resFirst && resSecond && resThird && mgr.m_CurrentKeyboardKey == first && (mgr.m_CurrentKeyboardKey != mgr.m_PreviousKeyboardKey));
}
const bool Engine::isKeyUp(const KeyboardKey::Key key){ 
    return !Engine::isKeyDown(key); 
}
const bool Engine::isMouseButtonDown(const MouseButton::Button button){
    return eventManager->m_MouseStatus.count(button);
}
const bool Engine::isMouseButtonDownOnce(const MouseButton::Button button){
    const bool res = Engine::isMouseButtonDown(button);
    auto& mgr = *eventManager;
    return (res && mgr.m_CurrentMouseButton == button && (mgr.m_CurrentMouseButton != mgr.m_PreviousMouseButton));
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
const double Engine::getMouseWheelDelta() {
    return Engine::getMouseWheelDelta(Resources::getWindow());
}
const double Engine::getMouseWheelDelta(Window& window){
    return window.getMouseWheelDelta();
}
void Engine::setMousePosition(Window& window, const float x, const float y, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)), window.getSFMLHandle());
    window.updateMousePosition(x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const unsigned int x, const unsigned int y, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x, y), window.getSFMLHandle());
    window.updateMousePosition(static_cast<float>(x), static_cast<float>(y), resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::vec2& pos, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(pos.x), static_cast<int>(pos.y)), window.getSFMLHandle());
    window.updateMousePosition(pos.x, pos.y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(Window& window, const glm::uvec2& pos, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x, pos.y), window.getSFMLHandle());
    window.updateMousePosition(static_cast<float>(pos.x), static_cast<float>(pos.y), resetDifference, resetPreviousPosition);
}


void Engine::setMousePosition(const float x, const float y, const bool resetDifference, const bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const unsigned int x, const unsigned int y, const bool resetDifference, const bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::vec2& pos, const bool resetDifference, const bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::uvec2& pos, const bool resetDifference, const bool resetPreviousPosition) {
    Engine::setMousePosition(Resources::getWindow(), pos, resetDifference, resetPreviousPosition);
}



void Engine::events::dispatchEvent(const unsigned int eventType) {
    eventManager->m_EventDispatcher.dispatchEvent(eventType);
}