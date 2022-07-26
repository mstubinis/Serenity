#pragma once
#ifndef ENGINE_ENGINE_EVENTS_H
#define ENGINE_ENGINE_EVENTS_H

class Window;

#include <serenity/events/EventDispatcher.h>
#include <serenity/events/EventIncludes.h>

#include <serenity/input/Keyboard/KeyboardModule.h>
#include <serenity/input/Mouse/MouseModule.h>
#include <serenity/input/Joystick/JoystickModule.h>
#include <serenity/dependencies/glm.h>

namespace Engine::priv {
    class EventModule final {
        public:
            EventDispatcher   m_EventDispatcher;

            EventModule();
            EventModule(const EventModule&)                = delete;
            EventModule& operator=(const EventModule&)     = delete;
            EventModule(EventModule&&) noexcept            = delete;
            EventModule& operator=(EventModule&&) noexcept = delete;

            void postUpdate();
        };
};

#endif