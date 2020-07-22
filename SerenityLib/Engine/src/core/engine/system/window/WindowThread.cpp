#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/window/Window.h>

using namespace std;

Engine::priv::WindowThread::WindowThread(WindowData& data) : m_Data(data) {
}
Engine::priv::WindowThread::~WindowThread() {
    cleanup();
}
bool Engine::priv::WindowThread::operator==(const bool rhs) const {
    bool res = m_EventThread.get();
    return (rhs) ? res : !res;
}
Engine::priv::WindowThread::operator bool() const {
    return (bool)m_EventThread.get();
}
std::optional<sf::Event> Engine::priv::WindowThread::try_pop() {
    auto x = m_Queue.try_pop();
    return std::move(x);
}
void Engine::priv::WindowThread::push(WindowEventThreadOnlyCommands::Command command) {
    m_MainThreadToEventThreadQueue.push(command);
}
void Engine::priv::WindowThread::updateLoop() {
    sf::Event e;
    if (!m_Data.m_UndergoingClosing) {
        //if (m_Data.m_SFMLWindow.waitEvent(e)) {
        while (m_Data.m_SFMLWindow.pollEvent(e)) {
            m_Queue.push(e);
        }
    }
    while (!m_MainThreadToEventThreadQueue.empty()) {
        auto command_ptr = m_MainThreadToEventThreadQueue.try_pop();
        switch (*command_ptr) {
            case WindowEventThreadOnlyCommands::HideMouse: {
                m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                m_Data.m_Flags.add(Window_Flags::MouseVisible);
                break;
            }case WindowEventThreadOnlyCommands::ShowMouse: {
                m_Data.m_SFMLWindow.setMouseCursorVisible(true);
                m_Data.m_Flags.add(Window_Flags::MouseVisible);
                break;
            }case WindowEventThreadOnlyCommands::RequestFocus: {
                m_Data.m_SFMLWindow.requestFocus();
                break;
            }case WindowEventThreadOnlyCommands::KeepMouseInWindow: {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
                m_Data.m_Flags.add(Window_Flags::MouseGrabbed);
                break;
            }case WindowEventThreadOnlyCommands::FreeMouseFromWindow: {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
                m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
                break;
            }default: {
                break;
            }
        }
    }
}
void Engine::priv::WindowThread::startup(Window& super, const string& name) {
    auto lamda = [&]() {
        m_Data.m_SFMLWindow.create(m_Data.m_VideoMode, name, m_Data.m_Style, m_Data.m_SFContextSettings);
        if (!m_Data.m_IconFile.empty()) {
            super.setIcon(m_Data.m_IconFile);
        }
        m_Data.m_SFMLWindow.setActive(false);
        m_Data.m_UndergoingClosing = false;
        while (!m_Data.m_UndergoingClosing) {
            updateLoop();
        }
    };
    m_EventThread.reset(NEW std::thread(lamda));
}
void Engine::priv::WindowThread::cleanup() {
    if (m_EventThread && m_EventThread->joinable()) {
        m_EventThread->join();
    }
}
