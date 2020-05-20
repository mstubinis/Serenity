#pragma once
#ifndef ENGINE_NETWORKING_MODULE_H
#define ENGINE_NETWORKING_MODULE_H

#include <core/engine/networking/SocketManager.h>

namespace Engine::priv {
    class NetworkingModule {
        friend class ::Engine::Networking::SocketTCP;
        friend class ::Engine::Networking::SocketUDP;
        friend class ::Engine::Networking::ListenerTCP;
        private:
            SocketManager m_SocketManager;
        public:
            NetworkingModule();
            virtual ~NetworkingModule();

            virtual void update(const float dt);
    };
};

#endif