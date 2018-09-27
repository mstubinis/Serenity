#include "Engine_Events.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"

#include <boost/algorithm/string.hpp> 

#include <glm/vec2.hpp>

using namespace Engine;
using namespace std;

class epriv::EventManager::impl final{
    public:
        unordered_map<uint,bool> m_KeyStatus, m_MouseStatus;

        float m_Delta;
        glm::vec2 m_Position, m_Position_Previous, m_Difference;

        uint m_currentKey, m_previousKey;
        uint m_currentButton, m_previousButton;

        void _init(const char* name,uint& w,uint& h){
            _initVars();
        }
        void _initVars(){
            m_Delta = 0;
            m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

            m_currentKey = m_previousKey = sf::Keyboard::Unknown;
            m_currentButton = m_previousButton = 100;  //we will use 100 as the "none" key
        }
        void _destruct(){
            m_KeyStatus.clear();
            m_MouseStatus.clear();
        }
        void _setMousePositionInternal(float x,float y,bool& resetDifference,bool& resetPrevious){
            glm::vec2 newPos = glm::vec2(x,y);
            if(resetPrevious == true) m_Position_Previous = newPos;
            else                      m_Position_Previous = m_Position;
            m_Position = newPos;
            m_Difference.x += (m_Position.x - m_Position_Previous.x);
            m_Difference.y += (m_Position.y - m_Position_Previous.y);
            if(resetDifference == true) m_Difference = glm::vec2(0.0f);
        }

        bool _isKeyDown(KeyboardKey::Key& key){
            if(m_KeyStatus[key]) return true; return false;
        }
        bool _isKeyUp(KeyboardKey::Key& key){
            if(!m_KeyStatus[key]) return true; return false;
        }
        bool _isKeyDownOnce(KeyboardKey::Key& key){
            bool res = _isKeyDown(key);
            if(res && m_currentKey == key && (m_currentKey != m_previousKey)) return true; return false;
        }

        bool _isMouseButtonDown(MouseButton::Button& button){
            if(m_MouseStatus[button]) return true; return false;
        }
        bool _isMouseButtonDownOnce(MouseButton::Button& button){
            bool res = _isMouseButtonDown(button);
            if(res && m_currentButton == button && (m_currentButton != m_previousButton)) return true; return false;
        }

        void _onEventKeyPressed(uint& key){
            m_previousKey = m_currentKey;
            m_currentKey = key;
            m_KeyStatus[key] = true;
        }
        void _onEventKeyReleased(uint& key){
            m_previousKey = sf::Keyboard::Unknown;
            m_currentKey = sf::Keyboard::Unknown;
            m_KeyStatus[key] = false;
        }
        void _onEventMouseWheelMoved(int& delta){
            m_Delta += (delta * 10);
        }
        void _onEventMouseButtonPressed(uint& mouseButton){
            m_previousButton = m_currentButton;
            m_currentButton = mouseButton;
            m_MouseStatus[mouseButton] = true;
        }
        void _onEventMouseButtonReleased(uint& mouseButton){
            m_previousButton = 100; //we will use 100 as the "none" key
            m_currentButton = 100;  //we will use 100 as the "none" key
            m_MouseStatus[mouseButton] = false;
        }
        void _onResetEvents(){
            m_previousKey = sf::Keyboard::Unknown;
            m_currentKey = sf::Keyboard::Unknown;
            for(auto iterator:m_KeyStatus){ iterator.second = false; }
            for(auto iterator:m_MouseStatus){ iterator.second = false; }
            m_Delta *= (float)(0.97 * (1.0-Resources::dt()));

            m_Difference.x = 0.0f; m_Difference.y = 0.0f;
        }
};

epriv::EventManager::impl* eventManager;

epriv::EventManager::EventManager(const char* name,uint w,uint h):m_i(new impl){
    m_i->_init(name,w,h);
    eventManager = m_i.get();
}
epriv::EventManager::~EventManager(){ m_i->_destruct(); }
void epriv::EventManager::_onEventKeyPressed(uint& key){ m_i->_onEventKeyPressed(key); }
void epriv::EventManager::_onEventKeyReleased(uint& key){ m_i->_onEventKeyReleased(key); }
void epriv::EventManager::_onEventMouseButtonPressed(uint mouseButton){ m_i->_onEventMouseButtonPressed(mouseButton); }
void epriv::EventManager::_onEventMouseButtonReleased(uint mouseButton){ m_i->_onEventMouseButtonReleased(mouseButton); }
void epriv::EventManager::_onEventMouseWheelMoved(int& delta){ m_i->_onEventMouseWheelMoved(delta); }
void epriv::EventManager::_onResetEvents(){ m_i->_onResetEvents(); }
void epriv::EventManager::_setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){ m_i->_setMousePositionInternal(x,y,resetDifference,resetPreviousPosition); }
bool Engine::isKeyDown(KeyboardKey::Key key){ return eventManager->_isKeyDown(key); }
bool Engine::isKeyDownOnce(KeyboardKey::Key key){ return eventManager->_isKeyDownOnce(key); }
bool Engine::isKeyUp(KeyboardKey::Key key){ return eventManager->_isKeyUp(key); }
bool Engine::isMouseButtonDown(MouseButton::Button button){ return eventManager->_isMouseButtonDown(button); }
bool Engine::isMouseButtonDownOnce(MouseButton::Button button){ return eventManager->_isMouseButtonDownOnce(button); }
const glm::vec2& Engine::getMouseDifference(){ return eventManager->m_Difference; }
const glm::vec2& Engine::getMousePositionPrevious(){ return eventManager->m_Position_Previous; }
const glm::vec2& Engine::getMousePosition(){ return eventManager->m_Position; }
const float Engine::getMouseWheelDelta(){ return eventManager->m_Delta; }
void Engine::setMousePosition(float x,float y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(x),int(y)),*Resources::getWindow()->getSFMLHandle());
    eventManager->_setMousePositionInternal(x,y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(uint x,uint y,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(x,y),*Resources::getWindow()->getSFMLHandle());
    eventManager->_setMousePositionInternal((float)x,(float)y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::vec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(int(pos.x),int(pos.y)),*Resources::getWindow()->getSFMLHandle());
    eventManager->_setMousePositionInternal(pos.x,pos.y,resetDifference,resetPreviousPosition);
}
void Engine::setMousePosition(glm::uvec2 pos,bool resetDifference,bool resetPreviousPosition){
    sf::Mouse::setPosition(sf::Vector2i(pos.x,pos.y),*Resources::getWindow()->getSFMLHandle());
    eventManager->_setMousePositionInternal((float)pos.x,(float)pos.y,resetDifference,resetPreviousPosition);
}
