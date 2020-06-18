#pragma once
#ifndef ENGINE_EVENT_H
#define ENGINE_EVENT_H

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
#include <core/engine/events/EventIncludes.h>
#include <core/engine/networking/SocketInterface.h>

namespace Engine::priv{

    struct EventSocket final {
        unsigned short   localPort  = 0;
        unsigned short   remotePort = 0;
        sf::IpAddress    remoteIP   = sf::IpAddress::None;
        SocketType::Type type       = SocketType::Type::Unknown;

        EventSocket() = default;
        EventSocket(unsigned short localPort_, unsigned short remotePort_, sf::IpAddress remoteIP_, SocketType::Type type_) {
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
        EventWindowResized(unsigned int width_, unsigned int height_) {
            width = width_;
            height = height_;
        }
    };
    struct EventWindowFullscreenChanged final{ 
        bool isFullscreen = false;
        EventWindowFullscreenChanged() = default;
        EventWindowFullscreenChanged(bool isFullscreen_) {
            isFullscreen = isFullscreen_;
        }
    };
    struct EventKeyboard final{ 
        KeyboardKey::Key  key;
        bool              alt, control, shift, system;
        EventKeyboard() = default;
        EventKeyboard(KeyboardKey::Key key_, bool alt_, bool control_, bool shift_, bool system_) {
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
        EventTextEntered(std::uint32_t unicode_) {
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
        MouseButton::Button button; 
        float x, y; 
        EventMouseButton() = default;
        EventMouseButton(MouseButton::Button button_, float x_, float y_) {
            button = button_;
            x      = x_;
            y      = y_;
        }
    };
    struct EventMouseMove final{ 
        float x,y;
        EventMouseMove() = default;
        EventMouseMove(float x_, float y_) {
            x = x_;
            y = y_;
        }
    };
    struct EventMouseWheel final{ 
        float delta;
        int x, y;
        EventMouseWheel() = default;
        EventMouseWheel(float delta_, int x_, int y_) {
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
        EventJoystickMoved(unsigned int joystickID_, JoystickAxis::Axis axis_, float position_) {
            joystickID = joystickID_;
            axis       = axis_;
            position   = position_;
        }
    };
    struct EventJoystickButton final{ 
        unsigned int  joystickID = 0;
        unsigned int  button     = 0;
        EventJoystickButton() = default;
        EventJoystickButton(unsigned int joystickID_, unsigned int button_) {
            joystickID = joystickID_;
            button     = button_;
        }
    };
    struct EventJoystickConnection final{ 
        unsigned int  joystickID = 0;
        EventJoystickConnection() = default;
        EventJoystickConnection(unsigned int joystickID_) {
            joystickID = joystickID_;
        }
    };
    struct EventSoundStatusChanged final {
        unsigned int  status = 0;
        EventSoundStatusChanged() = default;
        EventSoundStatusChanged(unsigned int status_) {
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
    Event(EventType::Type type_) {
        type = type_;
    }

};

#endif