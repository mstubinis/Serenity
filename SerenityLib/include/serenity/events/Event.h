#pragma once
#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

class  Window;
class  Scene;
class  ResourceBaseClass;
namespace sf {
    class Packet;
}
namespace Engine::Networking {
    class Server;
    class ServerClient;
    class ServerThread;
}
#include <SFML/Network/IpAddress.hpp>
#include <serenity/networking/ISocket.h>
#include <serenity/networking/Packet.h>
#include <serenity/events/EventModule.h>

namespace Engine::priv {
    struct EventEnum final {
        uint32_t enumValue = 0;
        void*    enumPtr   = nullptr;
        EventEnum() = default;
        EventEnum(uint32_t enum_)
            : enumValue{ enum_ }
        {}
        EventEnum(uint32_t enum_, void* enumPtr_)
            : enumValue{ enum_ }
            , enumPtr{ enumPtr_ }
        {}
    };
    struct EventSocket final {
        sf::IpAddress  remoteIP   = sf::IpAddress::None;
        uint16_t       localPort  = 0;
        uint16_t       remotePort = 0;
        uint8_t        type       = SocketType::Unknown;

        EventSocket() = default;
        EventSocket(uint16_t localPort_, uint16_t remotePort_, sf::IpAddress remoteIP_, SocketType type_)
            : remoteIP  { remoteIP_ }
            , localPort { localPort_ }
            , remotePort{ remotePort_ }
            , type      { type_ }
        {
        }
        [[nodiscard]] inline const std::string ipToString() const { 
            return remoteIP.toString(); 
        }
    };
    struct EventServer final {
        Engine::Networking::Server* server = nullptr;
        EventServer() = default;
        EventServer(Engine::Networking::Server& server_) 
            : server{ &server_ }
        {}
    };
    struct EventClient final {
        Engine::Networking::ServerClient* client = nullptr;
        EventClient() = default;
        EventClient(Engine::Networking::ServerClient& client_) 
            : client{ &client_ }
        {}
    };
    struct EventResource final {
        ResourceBaseClass* resource = nullptr;
        EventResource() = default;
        EventResource(ResourceBaseClass* resource_)
            : resource{ resource_ }
        {}
    };
    struct EventWindowResized final{ 
        uint32_t  width  = 0;
        uint32_t  height = 0;
        EventWindowResized() = default;
        EventWindowResized(uint32_t width_, uint32_t height_)
            : width{ width_ }
            , height{ height_ }
        {}
    };
    struct EventWindowFullscreenChanged final{ 
        bool isFullscreen = false;
        EventWindowFullscreenChanged() = default;
        EventWindowFullscreenChanged(bool isFullscreen_) {
            isFullscreen = isFullscreen_;
        }
    };
    struct EventKeyboard final{ 
        uint32_t          key;
        bool              alt, control, shift, system;
        EventKeyboard() = default;
        EventKeyboard(KeyboardKey key_, bool alt_ = false, bool control_ = false, bool shift_ = false, bool system_ = false)
            : key    { key_ }
            , alt    { alt_ }
            , control{ control_ }
            , shift  { shift_ }
            , system { system_ }
        {}
    };
    struct EventTextEntered final{ 
        uint32_t  unicode;
        EventTextEntered() = default;
        EventTextEntered(uint32_t unicode_) 
            : unicode{ unicode_ }
        {}
        const std::string convert() const {
            if (unicode == 27 || unicode == 13 || unicode == 9 || unicode == 8) { //27 = esc, 13 = enter, 9 = tab, 8 = backspace
                return "";
            }
            wchar_t wchar = (wchar_t)unicode;
            std::wstring ws(&wchar);
            std::string res;
            std::for_each(std::cbegin(ws), std::cend(ws), [&res](const auto c) {
                res += (char)c;
            });
            res = res[0]; //this is to remove garbage characters, which are from the wide string
            return res;
        }
    };
    struct EventMouseButton final{ 
        uint32_t  button  = MouseButton::Unknown;
        float     x       = 0.0f;
        float     y       = 0.0f;
        EventMouseButton() = default;
        EventMouseButton(MouseButton button_, float x_, float y_) 
            : button{ button_ }
            , x{ x_ }
            , y{ y_ }
        {}
        EventMouseButton(MouseButton button_, Window& window)
            : button{ button_ }
        {
            const auto& m = Engine::getMousePosition(window);
            x = m.x;
            y = m.y;
        }
    };
    struct EventMouseMove final{ 
        float x = 0.0f;
        float y = 0.0f;
        EventMouseMove() = default;
        EventMouseMove(float x_, float y_) 
            : x{ x_ }
            , y{ y_ }
        {}
        EventMouseMove(Window& window) {
            const auto& m = Engine::getMousePosition(window);
            x = m.x;
            y = m.y;
        }
    };
    struct EventMouseWheel final{ 
        float delta = 0.0f;
        int   x     = 0;
        int   y     = 0;
        EventMouseWheel() = default;
        EventMouseWheel(float delta_, int x_, int y_) 
            : delta{ delta_ }
            , x{ x_ }
            , y{ y_ }
        {}
    };
    struct EventJoystickMoved final{ 
        uint32_t            joystickID = 0;
        uint32_t            axis       = JoystickAxis::Unknown;
        float               position   = 0.0f;
        EventJoystickMoved() = default;
        EventJoystickMoved(uint32_t joystickID_, JoystickAxis axis_, float position_)
            : joystickID{ joystickID_ }
            , axis{ axis_ }
            , position{ position_ }

        {}
    };
    struct EventJoystickButton final {
        uint32_t  joystickID = 0;
        uint32_t  button     = 0;
        EventJoystickButton() = default;
        EventJoystickButton(uint32_t joystickID_, uint32_t button_)
            : joystickID{ joystickID_ }
            , button{ button_ }
        {}
    };
    struct EventJoystickConnection final{ 
        uint32_t  joystickID = 0;
        EventJoystickConnection() = default;
        EventJoystickConnection(uint32_t joystickID_)
            : joystickID{ joystickID_ }
        {}
    };
    struct EventSoundStatusChanged final {
        uint32_t  status = 0;
        EventSoundStatusChanged() = default;
        EventSoundStatusChanged(uint32_t status_)
            : status{ status_ }
        {}
    };
    struct EventSceneChanged final{ 
        Scene* oldScene = nullptr;
        Scene* newScene = nullptr;
        EventSceneChanged() = default;
        EventSceneChanged(Scene* old_, Scene* new_) 
            : oldScene{ old_ }
            , newScene{ new_ }
        {}
    };
};
struct Event final{
    uint32_t type = EventType::Unknown;
    union {
        Engine::priv::EventEnum                       eventEnum{};
        Engine::priv::EventWindowResized              eventWindowResized;
        Engine::priv::EventWindowFullscreenChanged    eventWindowFullscreenChanged;
        Engine::priv::EventKeyboard                   eventKeyboard;
        Engine::priv::EventTextEntered                eventTextEntered;
        Engine::priv::EventMouseButton                eventMouseButton;
        Engine::priv::EventMouseMove                  eventMouseMoved;
        Engine::priv::EventMouseWheel                 eventMouseWheel;
        Engine::priv::EventJoystickMoved              eventJoystickMoved;
        Engine::priv::EventJoystickButton             eventJoystickButton;
        Engine::priv::EventJoystickConnection         eventJoystickConnection;
        Engine::priv::EventSoundStatusChanged         eventSoundStatusChanged;
        Engine::priv::EventSceneChanged               eventSceneChanged;
        Engine::priv::EventResource                   eventResource;

        Engine::priv::EventSocket                     eventSocket;
        Engine::priv::EventServer                     eventServer;
        Engine::priv::EventClient                     eventClient;
    };
    Event() = delete;
    ~Event() {}
    Event(EventType type_) 
        : type{ type_ }
    {}

    Event(EventType type_, Engine::priv::EventEnum&& eventEnum_)
        : Event{ type_ }
    {
        eventEnum = std::move(eventEnum_);
    }
    Event(EventType type_, Engine::priv::EventWindowResized&& eventWindowResized_)
        : Event{ type_ }
    {
        eventWindowResized = std::move(eventWindowResized_);
    }
    Event(EventType type_, Engine::priv::EventWindowFullscreenChanged&& eventWindowFullscreenChanged_)
        : Event{ type_ }
    {
        eventWindowFullscreenChanged = std::move(eventWindowFullscreenChanged_);
    }
    Event(EventType type_, Engine::priv::EventKeyboard&& eventKeyboard_)
        : Event{ type_ }
    {
        eventKeyboard = std::move(eventKeyboard_);
    }
    Event(EventType type_, Engine::priv::EventTextEntered&& eventTextEntered_)
        : Event{ type_ }
    {
        eventTextEntered = std::move(eventTextEntered_);
    }
    Event(EventType type_, Engine::priv::EventMouseButton&& eventMouseButton_)
        : Event{ type_ }
    {
        eventMouseButton = std::move(eventMouseButton_);
    }
    Event(EventType type_, Engine::priv::EventMouseMove&& eventMouseMoved_)
        : Event{ type_ }
    {
        eventMouseMoved = std::move(eventMouseMoved_);
    }
    Event(EventType type_, Engine::priv::EventMouseWheel&& eventMouseWheel_)
        : Event{ type_ }
    {
        eventMouseWheel = std::move(eventMouseWheel_);
    }
    Event(EventType type_, Engine::priv::EventJoystickMoved&& eventJoystickMoved_)
        : Event{ type_ }
    {
        eventJoystickMoved = std::move(eventJoystickMoved_);
    }
    Event(EventType type_, Engine::priv::EventJoystickButton&& eventJoystickButton_)
        : Event{ type_ }
    {
        eventJoystickButton = std::move(eventJoystickButton_);
    }
    Event(EventType type_, Engine::priv::EventJoystickConnection&& eventJoystickConnection_)
        : Event{ type_ }
    {
        eventJoystickConnection = std::move(eventJoystickConnection_);
    }
    Event(EventType type_, Engine::priv::EventSoundStatusChanged&& eventSoundStatusChanged_)
        : Event{ type_ }
    {
        eventSoundStatusChanged = std::move(eventSoundStatusChanged_);
    }
    Event(EventType type_, Engine::priv::EventSceneChanged&& eventSceneChanged_)
        : Event{ type_ }
    {
        eventSceneChanged = std::move(eventSceneChanged_);
    }
    Event(EventType type_, Engine::priv::EventResource&& eventResource_)
        : Event{ type_ }
    {
        eventResource = std::move(eventResource_);
    }
    Event(EventType type_, Engine::priv::EventSocket&& eventSocket_)
        : Event{ type_ }
    {
        eventSocket = std::move(eventSocket_);
    }
    Event(EventType type_, Engine::priv::EventServer&& eventServer_)
        : Event{ type_ }
    {
        eventServer = std::move(eventServer_);
    }
    Event(EventType type_, Engine::priv::EventClient&& eventClient_)
        : Event{ type_ }
    {
        eventClient = std::move(eventClient_);
    }
};

#endif