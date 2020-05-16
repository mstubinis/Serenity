#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_MANAGER_H
#define ENGINE_NETWORKING_SOCKET_MANAGER_H

#include <vector>
//#include <iostream>

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
            void internal_add_socket(TVECTOR& vec, TSOCKET* socket) {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        //std::cout << "Socket (" << socket << ") already found, could not add" << "\n";
                        return;
                    }
                }
                //std::cout << "Adding socket: (" << socket << ")\n";
                vec.push_back(socket);
            }

            template<typename TVECTOR, typename TSOCKET>
            void internal_remove_socket(TVECTOR& vec, TSOCKET* socket) {
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (vec[i] == socket) {
                        vec.erase(vec.begin() + i);
                        //std::cout << "Removing socket: (" << socket << ")\n";
                        return;
                    }
                }
                //std::cout << "Socket (" << socket << ") was not found, could not remove" << "\n";
            }

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
