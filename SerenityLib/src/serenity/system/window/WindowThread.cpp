
#include <serenity/system/window/Window.h>
#include <serenity/system/Macros.h>

Engine::priv::WindowThread::WindowThread(WindowData& data) 
    : m_Data{ data }
{}
void Engine::priv::WindowThread::internal_push(WindowEventThreadOnlyCommands command) {
    m_MainThreadToEventThreadQueue.push(command);
}
void Engine::priv::WindowThread::internal_process_command_queue() {
    while (!m_MainThreadToEventThreadQueue.empty()) {
        auto command_ptr = m_MainThreadToEventThreadQueue.front();
        m_MainThreadToEventThreadQueue.pop();
        switch (command_ptr) {
            case WindowEventThreadOnlyCommands::ShowMouse: {
                m_Data.m_SFMLWindow.setMouseCursorVisible(true);
                m_Data.m_Flags.add(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::HideMouse: {
                m_Data.m_SFMLWindow.setMouseCursorVisible(false);
                m_Data.m_Flags.remove(Window_Flags::MouseVisible);
                break;
            } case WindowEventThreadOnlyCommands::RequestFocus: {
                m_Data.m_SFMLWindow.requestFocus();
                break;
            } case WindowEventThreadOnlyCommands::KeepMouseInWindow: {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(true);
                m_Data.m_Flags.add(Window_Flags::MouseGrabbed);
                break;
            } case WindowEventThreadOnlyCommands::FreeMouseFromWindow: {
                m_Data.m_SFMLWindow.setMouseCursorGrabbed(false);
                m_Data.m_Flags.remove(Window_Flags::MouseGrabbed);
                break;
            }
        }
    }
}
void Engine::priv::WindowThread::internal_populate_sf_event_queue() {
    sf::Event e;
    if (!m_Data.m_UndergoingClosing) {
        while (m_Data.m_SFMLWindow.pollEvent(e)) {
            m_SFEventQueue.push(e);
        }
    }
}

Engine::priv::WindowThread::EventType Engine::priv::WindowThread::internal_try_pop() noexcept {
    #if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        return m_SFEventQueue.try_pop();
    #else
        if (!m_Data.m_UndergoingClosing) {
            internal_populate_sf_event_queue();
            internal_process_command_queue();
        }
        if (m_SFEventQueue.size() > 0) {
            auto x = m_SFEventQueue.front();
            m_SFEventQueue.pop();
            return x;
        }
        return EventType{};
    #endif
}

void Engine::priv::WindowThread::internal_thread_startup(Window& super, const std::string& name, boost::latch* bLatch) {
    auto update_lambda_loop = [this, &super, &name, &bLatch]() {
        m_Data.m_SFMLWindow.create(m_Data.m_VideoMode, name, m_Data.m_Style, m_Data.m_SFContextSettings);
        super.setIcon(m_Data.m_IconFile);
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        m_Data.m_SFMLWindow.setActive(false);
#endif
        m_Data.m_UndergoingClosing = false;
        if (bLatch) {
            bLatch->count_down();
        }
#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
        while (!m_Data.m_UndergoingClosing) {
            internal_populate_sf_event_queue();
            internal_process_command_queue();
        }
#endif
    };

#if defined(ENGINE_THREAD_WINDOW_EVENTS) && !defined(_APPLE_)
    m_EventThread.reset(NEW std::jthread{ std::move(update_lambda_loop) });
#else
    update_lambda_loop();
#endif
}
