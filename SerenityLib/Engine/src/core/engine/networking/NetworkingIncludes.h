#pragma once
#ifndef ENGINE_NETWORKING_INCLUDES_H
#define ENGINE_NETWORKING_INCLUDES_H

namespace Engine::Networking {
    class NetworkingHelpers final {
        public:
            static uint32_t generate_nonce() {
                std::random_device device;
                std::mt19937 mt(device());
                std::uniform_int_distribution<sf::Uint32> distribution;
                sf::Uint32 nonce = 0;
                do {
                    nonce = distribution(mt);
                } while (nonce == 0);
                ASSERT(nonce != 0, __FUNCTION__ << "(): nonce was 0!");
                return nonce;
            }
    };
}

#endif