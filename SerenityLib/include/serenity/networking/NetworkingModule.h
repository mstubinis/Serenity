#pragma once
#ifndef ENGINE_NETWORKING_MODULE_H
#define ENGINE_NETWORKING_MODULE_H

namespace Engine::priv {
    class EditorWindowScene;
}

#include <serenity/networking/SocketManager.h>

namespace Engine::priv {
    class NetworkingModule final {
        friend class ::Engine::Networking::SocketTCP;
        friend class ::Engine::Networking::SocketUDP;
        friend class ::Engine::Networking::ListenerTCP;
        friend class Engine::priv::EditorWindowScene;
        private:
            SocketManager m_SocketManager;
        public:
            void update(const float dt);
    };
};

#endif