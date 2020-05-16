#pragma once
#ifndef ENGINE_EVENT_OBJECT_H
#define ENGINE_EVENT_OBJECT_H

class  Scene;
class  Mesh;
class  Material;
class  Texture;
namespace sf {
    class Packet;
}
namespace Engine::Networking {
    class Server;
    class ServerClient;
    class ServerThread;
}
#include <SFML/Network/IpAddress.hpp>
#include <core/engine/events/Engine_EventIncludes.h>
#include <core/engine/networking/SocketInterface.h>

namespace Engine::priv{

    struct EventSocket final {
        unsigned short   localPort  = 0;
        unsigned short   remotePort = 0;
        sf::IpAddress    remoteIP   = sf::IpAddress::None;
        SocketType::Type type;

        EventSocket() = default;
        EventSocket(const unsigned short localPort_, const unsigned short remotePort_, const sf::IpAddress remoteIP_, const SocketType::Type type_) {
            localPort  = localPort_;
            remotePort = remotePort_;
            remoteIP   = remoteIP_;
            type       = type_;
        }
    };

    struct EventServer final {
        Engine::Networking::Server* server = nullptr;
        EventServer() = default;
        EventServer(Engine::Networking::Server& server_) {
            server = &server_;
        }
    };

    struct EventClient final {
        Engine::Networking::ServerClient* client = nullptr;
        EventClient() = default;
        EventClient(Engine::Networking::ServerClient& client_) {
            client = &client_;
        }
    };

    struct EventPacket final {
        sf::Packet* packet = nullptr;
        EventPacket() = default;
        EventPacket(sf::Packet* packet_) {
            packet = packet_;
        }
    };
    struct EventMeshLoaded final {
        Mesh* mesh = nullptr;
        EventMeshLoaded() = default;
        EventMeshLoaded(Mesh* mesh_) {
            mesh = mesh_;
        }
    };
    struct EventMaterialLoaded final {
        Material* material = nullptr;
        EventMaterialLoaded() = default;
        EventMaterialLoaded(Material* material_) {
            material = material_;
        }
    };
    struct EventTextureLoaded final {
        Texture* texture = nullptr;
        EventTextureLoaded() = default;
        EventTextureLoaded(Texture* texture_) {
            texture = texture_;
        }
    };

    struct EventWindowResized final{ 
        unsigned int  width  = 0;
        unsigned int  height = 0;
        EventWindowResized() = default;
        EventWindowResized(const unsigned int width_, const unsigned int height_) {
            width = width_;
            height = height_;
        }
    };
    struct EventWindowFullscreenChanged final{ 
        bool isFullscreen = false;
        EventWindowFullscreenChanged() = default;
        EventWindowFullscreenChanged(const bool isFullscreen_) {
            isFullscreen = isFullscreen_;
        }
    };
    struct EventKeyboard final{ 
        KeyboardKey::Key  key;
        bool              alt, control, shift, system;
        EventKeyboard() = default;
        EventKeyboard(const KeyboardKey::Key key_, const bool alt_, const bool control_, const bool shift_, const bool system_) {
            key     = key_;
            alt     = alt_;
            control = control_;
            shift   = shift_;
            system  = system_;
        }
    };
    struct EventTextEntered final{ 
        std::uint32_t  unicode;
        EventTextEntered() = default;
        EventTextEntered(const std::uint32_t unicode_) {
            unicode = unicode_;
        }
        const std::string convert() const {
            if (unicode == 27 || unicode == 13 || unicode == 9 || unicode == 8) { //27 = esc, 13 = enter, 9 = tab, 8 = backspace
                return "";
            }
            wchar_t c = static_cast<wchar_t>(unicode);
            std::wstring ws(&c);
            std::string res;
            for (auto& c : ws) {
                res += static_cast<char>(c);
            }
            res = res[0];
            return res;
        }
    };
    struct EventMouseButton final{ 
        MouseButton::Button button; float x, y; 
        EventMouseButton() = default;
        EventMouseButton(const MouseButton::Button button_, const float x_, const float y_) {
            button = button_;
            x      = x_;
            y      = y_;
        }
    };
    struct EventMouseMove final{ 
        float x,y;
        EventMouseMove() = default;
        EventMouseMove(const float x_, const float y_) {
            x = x_;
            y = y_;
        }
    };
    struct EventMouseWheel final{ 
        float delta;
        int x, y;
        EventMouseWheel() = default;
        EventMouseWheel(const float delta_, const int x_, const int y_) {
            delta = delta_;
            x = x_;
            y = y_;
        }
    };
    struct EventJoystickMoved final{ 
        unsigned int        joystickID = 0;
        JoystickAxis::Axis  axis       = JoystickAxis::Axis::Unknown;
        float               position   = 0.0f;
        EventJoystickMoved() = default;
        EventJoystickMoved(const unsigned int joystickID_, const JoystickAxis::Axis axis_, const float position_) {
            joystickID = joystickID_;
            axis       = axis_;
            position   = position_;
        }
    };
    struct EventJoystickButton final{ 
        unsigned int  joystickID = 0;
        unsigned int  button     = 0;
        EventJoystickButton() = default;
        EventJoystickButton(const unsigned int joystickID_, const unsigned int button_) {
            joystickID = joystickID_;
            button     = button_;
        }
    };
    struct EventJoystickConnection final{ 
        unsigned int  joystickID = 0;
        EventJoystickConnection() = default;
        EventJoystickConnection(const unsigned int joystickID_) {
            joystickID = joystickID_;
        }
    };
    struct EventSoundStatusChanged final {
        unsigned int  status = 0;
        EventSoundStatusChanged() = default;
        EventSoundStatusChanged(const unsigned int status_) {
            status = status_;
        }
    };
    struct EventSceneChanged final{ 
        Scene* oldScene = nullptr;
        Scene* newScene = nullptr;
        EventSceneChanged() = default;
        EventSceneChanged(Scene* old_, Scene* new_) {
            oldScene = old_; 
            newScene = new_;
        }
    };
};
struct Event final{
    EventType::Type type = EventType::Type::Unknown;
    union{
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
        Engine::priv::EventMeshLoaded                 eventMeshLoaded;
        Engine::priv::EventMaterialLoaded             eventMaterialLoaded;
        Engine::priv::EventTextureLoaded              eventTextureLoaded;

        Engine::priv::EventPacket                     eventPacket;
        Engine::priv::EventSocket                     eventSocket;
        Engine::priv::EventServer                     eventServer;
        Engine::priv::EventClient                     eventClient;
    };
    Event() = delete;
    Event(const EventType::Type& type_) {
        type = type_;
    }

};
/*
Inherit from this class to expose your class to events and event dispatching, specifically the following functions:
    void registerEvent(const EventType::Type& type)    -  register this object as an observer to the parameterized event type
    void unregisterEvent(const EventType::Type& type)  -  unregister this object as an observer to the parameterized event type
    virtual void onEvent(const Event& e)               -  execute this function when the parameter event occurs
*/
class EventObserver{
    public:
        virtual ~EventObserver(){}

        void registerEvent(const EventType::Type type);
        void unregisterEvent(const EventType::Type type);
        const bool isRegistered(const EventType::Type type) const;

        void registerEvent(const unsigned int type);
        void unregisterEvent(const unsigned int type);
        const bool isRegistered(const unsigned int type) const;

        virtual void onEvent(const Event& e) {}
};
#endif