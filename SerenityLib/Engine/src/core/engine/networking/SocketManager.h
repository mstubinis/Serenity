#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_MANAGER_H
#define ENGINE_NETWORKING_SOCKET_MANAGER_H

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

            template<typename TVECTOR, typename TSOCKET>
            void internal_add_socket(TVECTOR& vec, TSOCKET* socket) noexcept {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        return;
                    }
                }
                vec.push_back(socket);
            }

            template<typename TVECTOR, typename TSOCKET>
            void internal_remove_socket(TVECTOR& vec, TSOCKET* socket) noexcept {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        vec.erase(vec.begin() + i);
                        return;
                    }
                }
            }
        public:
            SocketManager();
            ~SocketManager();

            void update(const float dt);

            void add_tcp_socket(Engine::Networking::SocketTCP* tcpSocketPtr);
            void add_udp_socket(Engine::Networking::SocketUDP* udpSocketPtr);
            void add_tcp_listener(Engine::Networking::ListenerTCP* tcpListenerPtr);

            void remove_tcp_socket(Engine::Networking::SocketTCP* tcpSocketPtr);
            void remove_udp_socket(Engine::Networking::SocketUDP* udpSocketPtr);
            void remove_tcp_listener(Engine::Networking::ListenerTCP* tcpListenerPtr);
    };
}

#endif
