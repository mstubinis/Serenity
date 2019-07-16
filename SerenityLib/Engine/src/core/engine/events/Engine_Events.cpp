#include <core/engine/events/Engine_Events.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Window.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

EventManager* EventManager::m_EventManager = nullptr;

vector<std::string> KEY_MAP = []() {
    vector<std::string> m; m.resize(KeyboardKey::_TOTAL);
    m[KeyboardKey::A] = 'A';
    m[KeyboardKey::B] = 'B';
    m[KeyboardKey::C] = 'C';
    m[KeyboardKey::D] = 'D';
    m[KeyboardKey::E] = 'E';
    m[KeyboardKey::F] = 'F';
    m[KeyboardKey::G] = 'G';
    m[KeyboardKey::H] = 'H';
    m[KeyboardKey::I] = 'I';
    m[KeyboardKey::J] = 'J';
    m[KeyboardKey::K] = 'K';
    m[KeyboardKey::L] = 'L';
    m[KeyboardKey::M] = 'M';
    m[KeyboardKey::N] = 'N';
    m[KeyboardKey::O] = 'O';
    m[KeyboardKey::P] = 'P';
    m[KeyboardKey::Q] = 'Q';
    m[KeyboardKey::R] = 'R';
    m[KeyboardKey::S] = 'S';
    m[KeyboardKey::T] = 'T';
    m[KeyboardKey::U] = 'U';
    m[KeyboardKey::V] = 'V';
    m[KeyboardKey::W] = 'W';
    m[KeyboardKey::X] = 'X';
    m[KeyboardKey::Y] = 'Y';
    m[KeyboardKey::Z] = 'Z';

    m[KeyboardKey::Num0] = '0';
    m[KeyboardKey::Num1] = '1';
    m[KeyboardKey::Num2] = '2';
    m[KeyboardKey::Num3] = '3';
    m[KeyboardKey::Num4] = '4';
    m[KeyboardKey::Num5] = '5';
    m[KeyboardKey::Num6] = '6';
    m[KeyboardKey::Num7] = '7';
    m[KeyboardKey::Num8] = '8';
    m[KeyboardKey::Num9] = '9';

    m[KeyboardKey::Numpad0] = '0';
    m[KeyboardKey::Numpad1] = '1';
    m[KeyboardKey::Numpad2] = '2';
    m[KeyboardKey::Numpad3] = '3';
    m[KeyboardKey::Numpad4] = '4';
    m[KeyboardKey::Numpad5] = '5';
    m[KeyboardKey::Numpad6] = '6';
    m[KeyboardKey::Numpad7] = '7';
    m[KeyboardKey::Numpad8] = '8';
    m[KeyboardKey::Numpad9] = '9';

    m[KeyboardKey::LeftBracket] = '{';
    m[KeyboardKey::RightBracket] = '}';
    m[KeyboardKey::SemiColon] = ';';
    m[KeyboardKey::Comma] = ',';
    m[KeyboardKey::Period] = '.';

    m[KeyboardKey::Add] = '+';
    m[KeyboardKey::Subtract] = '-';
    m[KeyboardKey::Multiply] = '*';
    m[KeyboardKey::Divide] = '/';

    m[KeyboardKey::BackSlash] = '\\';
    m[KeyboardKey::Slash] = '/';
    m[KeyboardKey::Tilde] = '~';
    m[KeyboardKey::Quote] = '"';
    m[KeyboardKey::Equal] = '=';
    m[KeyboardKey::Dash] = '-';

    m[KeyboardKey::LeftArrow] = '<';
    m[KeyboardKey::RightArrow] = '>';
    m[KeyboardKey::UpArrow] = '^';
    m[KeyboardKey::DownArrow] = 'v';
    m[KeyboardKey::Space] = ' ';
    m[KeyboardKey::Return] = '\n';

    m[KeyboardKey::Escape] = "";
    m[KeyboardKey::LeftControl] = "";
    m[KeyboardKey::LeftShift] = "";
    m[KeyboardKey::LeftAlt] = "";
    m[KeyboardKey::LeftSystem] = "";
    m[KeyboardKey::RightControl] = "";
    m[KeyboardKey::RightShift] = "";
    m[KeyboardKey::RightAlt] = "";
    m[KeyboardKey::RightSystem] = "";
    m[KeyboardKey::Menu] = "";
    m[KeyboardKey::BackSpace] = "";
    m[KeyboardKey::Tab] = "";
    m[KeyboardKey::PageUp] = "";
    m[KeyboardKey::PageDown] = "";
    m[KeyboardKey::End] = "";
    m[KeyboardKey::Home] = "";
    m[KeyboardKey::Insert] = "";
    m[KeyboardKey::Delete] = "";
    m[KeyboardKey::F1] = "";
    m[KeyboardKey::F2] = "";
    m[KeyboardKey::F3] = "";
    m[KeyboardKey::F4] = "";
    m[KeyboardKey::F5] = "";
    m[KeyboardKey::F6] = "";
    m[KeyboardKey::F7] = "";
    m[KeyboardKey::F8] = "";
    m[KeyboardKey::F9] = "";
    m[KeyboardKey::F10] = "";
    m[KeyboardKey::F11] = "";
    m[KeyboardKey::F12] = "";
    m[KeyboardKey::F13] = "";
    m[KeyboardKey::F14] = "";
    m[KeyboardKey::F15] = "";
    m[KeyboardKey::Pause] = "";
 
    return m;
}();


EventManager::EventManager(const char* name, const uint w, const uint h):m_EventDispatcher(){
    m_Delta = 0;
    m_Position = m_Position_Previous = m_Difference = glm::vec2(0.0f);

    m_currentKey    = m_previousKey    = KeyboardKey::Unknown;
    m_currentButton = m_previousButton = MouseButton::Unknown;

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
const std::string& Engine::mapKey(const KeyboardKey::Key& key) {
    return KEY_MAP[key];
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
