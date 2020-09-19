#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/window/Window.h>

Engine::priv::WindowThread::WindowThread(WindowData& data) 
    : m_Data{ data }
{}
Engine::priv::WindowThread::~WindowThread() {
    internal_cleanup();
}
std::optional<sf::Event> Engine::priv::WindowThread::internal_try_pop() {
    return m_SFEventQueue.try_pop();
}
void Engine::priv::WindowThread::internal_push(WindowEventThreadOnlyCommands command) {
    m_MainThreadToEventThreadQueue.push(command);
}
void Engine::priv::WindowThread::internal_update_loop() {
    sf::Event e;
    if (!m_Data.m_UndergoingClosing) {
        //if (m_Data.m_SFMLWindow.waitEvent(e)) {
        while (m_Data.m_SFMLWindow.pollEvent(e)) {
            m_SFEventQueue.push(e);
        }
    }
    while (!m_MainThreadToEventThreadQueue.empty()) {
        auto command_ptr = m_MainThreadToEventThreadQueue.try_pop();
        switch (*command_ptr) {
            case WindowEventThreadOnlyCommands::HideMouse: {
                m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                m_Data.m_Flags.remove(Window_Flags::MouseVisible);
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
void Engine::priv::WindowThread::internal_startup(Window& super, const std::string& name) {
    m_EventThread.reset(NEW std::thread([this, &super, &name]() {
        m_Data.m_SFMLWindow.create(m_Data.m_VideoMode, name, m_Data.m_Style, m_Data.m_SFContextSettings);
        super.setIcon(m_Data.m_IconFile);
        bool successfulDeactivation = m_Data.m_SFMLWindow.setActive(false);
        m_Data.m_UndergoingClosing = false;
        while (!m_Data.m_UndergoingClosing) {
            internal_update_loop();
        }
    }));
}
void Engine::priv::WindowThread::internal_cleanup() {
    if (m_EventThread && m_EventThread->joinable()) {
        m_EventThread->join();
    }
}
