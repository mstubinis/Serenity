#pragma once
#ifndef ENGINE_SYSTEM_ENGINE_EVENT_HANDLER_H
#define ENGINE_SYSTEM_ENGINE_EVENT_HANDLER_H

class  Window;
struct Event;
class  GameCore;
namespace Engine::priv {
    class  WindowData;
    class  EngineCore;
    class  EventModule;
    class  RenderModule;
    class  ResourceManager;
    class  EditorCore;
}

#include <serenity/system/TypeDefs.h>

namespace Engine::priv {
    class EngineEventHandler final {
        friend class  Engine::priv::EngineCore;
        friend class  ::Window;
        friend class  Engine::priv::WindowData;
        private:
            EditorCore&       m_EditorCore;
            EventModule&      m_EventModule;
            RenderModule&     m_RenderModule;
            ResourceManager&  m_ResourceManager;

            void internal_dispatch_event(Event&&) noexcept;

            void internal_on_event_resize(Window&, uint32_t width, uint32_t height, bool saveSize, GameCore&);
            void internal_on_event_game_ended();
            void internal_on_event_window_closed(Window&);
            void internal_on_event_window_requested_closed(Window&);
            void internal_on_event_lost_focus(Window&, GameCore&);
            void internal_on_event_gained_focus(Window&, GameCore&);
            void internal_on_event_text_entered(Window&, uint32_t unicode);
            void internal_on_event_key_pressed(Window&, uint32_t key);
            void internal_on_event_key_released(Window&, uint32_t key);
            void internal_on_event_mouse_wheel_scrolled(Window&, float delta, int mouseWheelX, int mouseWheelY);
            void internal_on_event_mouse_button_pressed(Window&, uint32_t mouseButton);
            void internal_on_event_mouse_button_released(Window&, uint32_t mouseButton);
            void internal_on_event_mouse_moved(Window&, int mouseX, int mouseY, GameCore&);
            void internal_on_event_mouse_entered(Window&, GameCore&);
            void internal_on_event_mouse_left(Window&, GameCore&);
            void internal_on_event_joystick_button_pressed(Window&, uint32_t button, uint32_t id);
            void internal_on_event_joystick_button_released(Window&, uint32_t button, uint32_t id);
            void internal_on_event_joystick_moved(Window&, uint32_t id, float position, uint32_t axis);
            void internal_on_event_joystick_connected(Window&, uint32_t id);
            void internal_on_event_joystick_disconnected(Window&, uint32_t id);
        public:
            EngineEventHandler(EditorCore&, EventModule&, RenderModule&, ResourceManager&);

            void poll_events(Window&, GameCore&);
    };
}

#endif