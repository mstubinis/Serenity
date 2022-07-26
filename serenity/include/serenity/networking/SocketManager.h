#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_MANAGER_H
#define ENGINE_NETWORKING_SOCKET_MANAGER_H

namespace Engine::Networking {
    class  SocketTCP;
    class  SocketUDP;
    class  ListenerTCP;
}
namespace Engine::priv {
    class  EditorWindowSceneFunctions;
}

#include <vector>

namespace Engine::priv {
    class SocketManager final {
        friend class Engine::priv::EditorWindowSceneFunctions;
        private:
            std::vector<Engine::Networking::SocketTCP*>    m_TCPSockets;
            std::vector<Engine::Networking::SocketUDP*>    m_UDPSockets;
            std::vector<Engine::Networking::ListenerTCP*>  m_TCPListeners;

            template<class TVECTOR, class TSOCKET>
            void internal_add_socket(TVECTOR& vec, TSOCKET* socket) noexcept {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        return;
                    }
                }
                vec.push_back(socket);
            }
            template<class TVECTOR, class TSOCKET>
            void internal_remove_socket(TVECTOR& vec, TSOCKET* socket) noexcept {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        vec.erase(vec.begin() + i);
                        return;
                    }
                }
            }
        public:
            void update(const float dt);

            void add_tcp_socket(Engine::Networking::SocketTCP*);
            void add_udp_socket(Engine::Networking::SocketUDP*);
            void add_tcp_listener(Engine::Networking::ListenerTCP*);

            void remove_tcp_socket(Engine::Networking::SocketTCP*);
            void remove_udp_socket(Engine::Networking::SocketUDP*);
            void remove_tcp_listener(Engine::Networking::ListenerTCP*);

            [[nodiscard]] inline const std::vector<Engine::Networking::SocketTCP*>& getTCPSockets() const noexcept { return m_TCPSockets; };
            [[nodiscard]] inline const std::vector<Engine::Networking::SocketUDP*>& getUDPSockets() const noexcept { return m_UDPSockets; };
            [[nodiscard]] inline const std::vector<Engine::Networking::ListenerTCP*>& getTCPListeners() const noexcept { return m_TCPListeners; };
    };
}
#endif
