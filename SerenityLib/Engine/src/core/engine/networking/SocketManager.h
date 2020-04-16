#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_MANAGER_H
#define ENGINE_NETWORKING_SOCKET_MANAGER_H

#include <vector>

namespace Engine::Networking {
    class SocketTCP;
    class SocketUDP;
    class ListenerTCP;
}

namespace Engine::priv {
    class SocketManager final {
        private:
            std::vector<Engine::Networking::SocketTCP*>    m_TCPSockets;
            std::vector<Engine::Networking::SocketUDP*>    m_UDPSockets;
            std::vector<Engine::Networking::ListenerTCP*>  m_TCPListeners;
        public:
            SocketManager();
            ~SocketManager();

            void update(const float dt);

            void add_tcp_socket(Engine::Networking::SocketTCP*);
            void add_udp_socket(Engine::Networking::SocketUDP*);
            void add_tcp_listener(Engine::Networking::ListenerTCP*);

            void remove_tcp_socket(Engine::Networking::SocketTCP*);
            void remove_udp_socket(Engine::Networking::SocketUDP*);
            void remove_tcp_listener(Engine::Networking::ListenerTCP*);
    };
}

#endif
