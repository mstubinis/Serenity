#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Window.h>

using namespace Engine;
using namespace std;

epriv::EventManager* epriv::EventManager::m_EventManager = nullptr;

epriv::EventManager::EventManager(const char* name,uint w,uint h):m_EventDispatcher(){
    m_Delta = 0;
    m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

    m_currentKey = m_previousKey = KeyboardKey::Unknown;
    m_currentButton = m_previousButton = MouseButton::Unknown;

    EventManager::m_EventManager = this;
}
epriv::EventManager::~EventManager(){
    m_KeyStatus.clear();
    m_MouseStatus.clear();
    EventManager::m_EventManager = nullptr;
}

void epriv::EventManager::setMousePositionInternal(float x, float y, bool resetDifference, bool resetPrevious) {
    glm::vec2 newPos = glm::vec2(x, y);
    resetPrevious ? m_Position_Previous = newPos : m_Position_Previous = m_Position;
    m_Position = newPos;
    m_Difference += (m_Position - m_Position_Previous);
    if (resetDifference) 
        m_Difference = glm::vec2(0.0f);
}

void epriv::EventManager::onEventKeyPressed(uint& key){ 
    m_previousKey = m_currentKey;
    m_currentKey = key;
    m_KeyStatus[key] = true;
}
void epriv::EventManager::onEventKeyReleased(uint& key){ 
    m_previousKey = KeyboardKey::Unknown;
    m_currentKey = KeyboardKey::Unknown;
    m_KeyStatus[key] = false;
}
void epriv::EventManager::onEventMouseButtonPressed(uint& mouseButton){ 
    m_previousButton = m_currentButton;
    m_currentButton = mouseButton;
    m_MouseStatus[mouseButton] = true;
}
void epriv::EventManager::onEventMouseButtonReleased(uint& mouseButton){ 
    m_previousButton = MouseButton::Unknown;
    m_currentButton = MouseButton::Unknown;
    m_MouseStatus[mouseButton] = false;
}
void epriv::EventManager::onEventMouseWheelMoved(int& delta){ 
    m_Delta += (delta * 10);
}
void epriv::EventManager::onResetEvents(const float& dt){ 
    m_previousKey = KeyboardKey::Unknown;
    m_currentKey = KeyboardKey::Unknown;

    float boost = ((1.0f / dt)) * 0.003f;
    float step = (1.0f - dt);
    m_Delta *= step * step * step;

    m_Difference.x = 0.0f;
    m_Difference.y = 0.0f;
}
const bool Engine::isKeyDown(KeyboardKey::Key key){
    return (!epriv::EventManager::m_EventManager->m_KeyStatus[key]) ? false : true;
}
const bool Engine::isKeyDownOnce(KeyboardKey::Key key){
    bool res = Engine::isKeyDown(key);
    auto& mgr = *epriv::EventManager::m_EventManager;
    return (res && mgr.m_currentKey == key && (mgr.m_currentKey != mgr.m_previousKey)) ? true : false;
}

const bool Engine::isKeyDownOnce(KeyboardKey::Key first, KeyboardKey::Key second) {
    bool resFirst = Engine::isKeyDown(first);
    bool resSecond = Engine::isKeyDown(second);
    auto& mgr = *epriv::EventManager::m_EventManager;
    return ( resFirst && resSecond && mgr.m_currentKey == second && (mgr.m_currentKey != mgr.m_previousKey)) ? true : false;
}

const bool Engine::isKeyUp(KeyboardKey::Key key){ return !Engine::isKeyDown(key); }
const bool Engine::isMouseButtonDown(MouseButton::Button button){
    return (!epriv::EventManager::m_EventManager->m_MouseStatus[button]) ? false : true;
}
const bool Engine::isMouseButtonDownOnce(MouseButton::Button button){
    bool res = Engine::isMouseButtonDown(button);
    auto& mgr = *epriv::EventManager::m_EventManager;
    return (res && mgr.m_currentButton == button && (mgr.m_currentButton != mgr.m_previousButton)) ? true : false;
}
const glm::vec2& Engine::getMouseDifference(){ return epriv::EventManager::m_EventManager->m_Difference; }
const glm::vec2& Engine::getMousePositionPrevious(){ return epriv::EventManager::m_EventManager->m_Position_Previous; }
const glm::vec2& Engine::getMousePosition(){ return epriv::EventManager::m_EventManager->m_Position; }
const float Engine::getMouseWheelDelta(){ return epriv::EventManager::m_EventManager->m_Delta; }
void Engine::setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(x),int(y)),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal(x,y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(uint x,uint y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x,y),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal((float)x,(float)y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::vec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(pos.x),int(pos.y)),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal(pos.x,pos.y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::uvec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x,pos.y),Resources::getWindow().getSFMLHandle());
    epriv::EventManager::m_EventManager->setMousePositionInternal((float)pos.x,(float)pos.y,resetDifference,resetPreviousPosition);
}
