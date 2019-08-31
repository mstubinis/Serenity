#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Window.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

EventManager* EventManager::m_EventManager = nullptr;

EventManager::EventManager(){
    m_Delta = 0;
    m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

    m_currentKey    = m_previousKey     = KeyboardKey::Unknown;
    m_currentButton = m_previousButton  = MouseButton::Unknown;

    m_EventManager = this;
}
EventManager::~EventManager(){
    m_KeyStatus.clear();
    m_MouseStatus.clear();
    m_EventManager = nullptr;
}

void EventManager::setMousePositionInternal(const float x, const float y, const bool resetDifference, const bool resetPrevious) {
    const glm::vec2 newPos = glm::vec2(x, Resources::getWindowSize().y - y); //opengl flipping y axis
    resetPrevious ? m_Position_Previous = newPos : m_Position_Previous = m_Position;
    m_Position = newPos;
    m_Difference += (m_Position - m_Position_Previous);
    if (resetDifference) {
        m_Difference = glm::vec2(0.0f);
    }
}

void EventManager::onEventKeyPressed(const uint& key){
    m_previousKey    = m_currentKey;
    m_currentKey     = key;
    m_KeyStatus[key] = true;
}
void EventManager::onEventKeyReleased(const uint& key){
    m_previousKey    = KeyboardKey::Unknown;
    m_currentKey     = KeyboardKey::Unknown;
    m_KeyStatus[key] = false;
}
void EventManager::onEventMouseButtonPressed(const uint& mouseButton){
    m_previousButton           = m_currentButton;
    m_currentButton            = mouseButton;
    m_MouseStatus[mouseButton] = true;
}
void EventManager::onEventMouseButtonReleased(const uint& mouseButton){
    m_previousButton           = MouseButton::Unknown;
    m_currentButton            = MouseButton::Unknown;
    m_MouseStatus[mouseButton] = false;
}
void EventManager::onEventMouseWheelMoved(const int& delta){
    m_Delta += (static_cast<double>(delta) * 10.0);
}
void EventManager::onResetEvents(const double& dt){
    m_previousKey    = KeyboardKey::Unknown;
    m_currentKey     = KeyboardKey::Unknown;
    m_previousButton = MouseButton::Unknown;
    m_currentButton  = MouseButton::Unknown;

    const double step = (1.0 - dt);
    m_Delta *= step * step * step;

    m_Difference.x = 0.0f;
    m_Difference.y = 0.0f;
}
const KeyboardKey::Key Engine::getPressedKey() {
    return static_cast<KeyboardKey::Key>(EventManager::m_EventManager->m_currentKey);
}
const MouseButton::Button Engine::getPressedButton() {
    return static_cast<MouseButton::Button>(EventManager::m_EventManager->m_currentButton);
}
const bool Engine::isKeyDown(const KeyboardKey::Key key){
    return (!EventManager::m_EventManager->m_KeyStatus[key]) ? false : true;
}
const bool Engine::isKeyDownOnce() {
    auto& mgr = *EventManager::m_EventManager;
    return (mgr.m_currentKey != mgr.m_previousKey) ? true : false;
}


const bool Engine::isKeyDownOnce(const KeyboardKey::Key key){
    const bool res = Engine::isKeyDown(key);
    auto& mgr = *EventManager::m_EventManager;
    return (res && mgr.m_currentKey == key && (mgr.m_currentKey != mgr.m_previousKey)) ? true : false;
}

const bool Engine::isKeyDownOnce(const KeyboardKey::Key first, const KeyboardKey::Key second) {
    const bool& resFirst = Engine::isKeyDown(first);
    const bool& resSecond = Engine::isKeyDown(second);
    auto& mgr = *EventManager::m_EventManager;
    return ( resFirst && resSecond && mgr.m_currentKey == second && (mgr.m_currentKey != mgr.m_previousKey)) ? true : false;
}

const bool Engine::isKeyUp(const KeyboardKey::Key key){ 
    return !Engine::isKeyDown(key); 
}
const bool Engine::isMouseButtonDown(const MouseButton::Button button){
    return (!EventManager::m_EventManager->m_MouseStatus[button]) ? false : true;
}
const bool Engine::isMouseButtonDownOnce(const MouseButton::Button button){
    const bool res = Engine::isMouseButtonDown(button);
    auto& mgr = *EventManager::m_EventManager;
    return (res && mgr.m_currentButton == button && (mgr.m_currentButton != mgr.m_previousButton)) ? true : false;
}
const glm::vec2& Engine::getMouseDifference(){ 
    return EventManager::m_EventManager->m_Difference; 
}
const glm::vec2& Engine::getMousePositionPrevious(){ 
    return EventManager::m_EventManager->m_Position_Previous; 
}
const glm::vec2& Engine::getMousePosition(){ 
    return EventManager::m_EventManager->m_Position; 
}
const double Engine::getMouseWheelDelta(){ 
    return EventManager::m_EventManager->m_Delta; 
}
void Engine::setMousePosition(const float x, const float y, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)), Resources::getWindow().getSFMLHandle());
    EventManager::m_EventManager->setMousePositionInternal(x, y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const uint x, const uint y, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x, y), Resources::getWindow().getSFMLHandle());
    EventManager::m_EventManager->setMousePositionInternal(static_cast<float>(x), static_cast<float>(y), resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::vec2& pos, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(pos.x), static_cast<int>(pos.y)), Resources::getWindow().getSFMLHandle());
    EventManager::m_EventManager->setMousePositionInternal(pos.x, pos.y, resetDifference, resetPreviousPosition);
}
void Engine::setMousePosition(const glm::uvec2& pos, const bool resetDifference, const bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x, pos.y), Resources::getWindow().getSFMLHandle());
    EventManager::m_EventManager->setMousePositionInternal(static_cast<float>(pos.x), static_cast<float>(pos.y), resetDifference, resetPreviousPosition);
}
