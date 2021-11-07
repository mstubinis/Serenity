#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

#include <SFML/Network/Packet.hpp>
#include <serenity/system/TypeDefs.h>
#include <serenity/dependencies/glm.h>
#include <serenity/system/Macros.h>
#include <serenity/ecs/entity/Entity.h>
#include <array>
#include <vector>
#include <functional>

using PacketSequence  = uint16_t;
using PacketBitfield  = uint32_t;
using PacketTimestamp = uint32_t;

namespace Engine::Networking {
    class Server;
    class ServerClient;
};

sf::Packet& operator <<(sf::Packet&, Entity) noexcept;
sf::Packet& operator >>(sf::Packet&, Entity) noexcept;

//glm vector floats
sf::Packet& operator <<(sf::Packet&, const glm::vec2&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::vec3&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::vec4&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::vec2&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::vec3&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::vec4&) noexcept;
//glm vector ints
sf::Packet& operator <<(sf::Packet&, const glm::ivec2&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::ivec3&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::ivec4&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::ivec2&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::ivec3&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::ivec4&) noexcept;
//glm vector uints
sf::Packet& operator <<(sf::Packet&, const glm::uvec2&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::uvec3&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::uvec4&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::uvec2&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::uvec3&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::uvec4&) noexcept;
//glm vector doubles
sf::Packet& operator <<(sf::Packet&, const glm::dvec2&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::dvec3&) noexcept;
sf::Packet& operator <<(sf::Packet&, const glm::dvec4&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::dvec2&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::dvec3&) noexcept;
sf::Packet& operator >>(sf::Packet&, glm::dvec4&) noexcept;

template<class T, size_t arraySize>
inline sf::Packet& operator <<(sf::Packet& packet, const std::array<T, arraySize>& data) noexcept {
    for (size_t i = 0; i < arraySize; ++i) {
        packet << data[i];
    }
    return packet;
}
template<class T, size_t arraySize>
inline sf::Packet& operator >>(sf::Packet& packet, std::array<T, arraySize>& data) noexcept {
    for (size_t i = 0; i < arraySize; ++i) {
        packet >> data[i];
    }
    return packet;
}


template<class T>
inline sf::Packet& operator <<(sf::Packet& packet, const std::vector<T>& vector) noexcept {
    sf::Uint32 vecSize = static_cast<sf::Uint32>(vector.size());
    packet << vecSize;
    for (sf::Uint32 i = 0; i < vecSize; ++i) {
        packet << vector[i];
    }
    return packet;
}
template<class T>
inline sf::Packet& operator >>(sf::Packet& packet, std::vector<T>& vector) noexcept {
    sf::Uint32 vecSize;
    packet >> vecSize;
    vector.reserve(vecSize);
    T item;
    for (sf::Uint32 i = 0; i < vecSize; ++i) {
        packet >> item;
        vector.push_back(item);
    }
    return packet;
}



namespace Engine::Networking {
    class Packet : public sf::Packet {
        public:
            template<class T> static inline constexpr bool sequence_greater_than(T s1, T s2) noexcept {
                return ((s1 > s2) && (s1 - s2 <= std::numeric_limits<T>().max())) || ((s1 < s2) && (s2 - s1 > std::numeric_limits<T>().max()));
            }
        public:
            uint32_t           m_PacketType     = 0;
            PacketTimestamp    m_Timestamp      = 0;
            PacketBitfield     m_AckBitfield    = 0;
            PacketSequence     m_SequenceNumber = 0;
            PacketSequence     m_Ack            = 0;
            bool               m_Valid          = false;
        public:
            Packet() = default;
            Packet(uint32_t PacketType)
                : m_PacketType { PacketType }
            {}
            Packet(sf::Packet&)                  = delete;

            Packet& operator=(const Packet&)     = default;
            Packet(const Packet&)                = default;
            Packet(Packet&&) noexcept            = default;
            Packet& operator=(Packet&&) noexcept = default;
            virtual ~Packet() {}

            inline constexpr bool isNull() const noexcept { return m_Timestamp == 0 || m_PacketType == 0 || m_Valid == false; }

            void initial_build() {
                (*this
                    << m_PacketType
                    << m_Timestamp
                    << m_AckBitfield
                    << m_SequenceNumber
                    << m_Ack
                );
            }
            void initial_unpack() {
                if (!sf::Packet::endOfPacket()) {
                    m_Valid = (*this
                        >> m_PacketType
                        >> m_Timestamp
                        >> m_AckBitfield
                        >> m_SequenceNumber
                        >> m_Ack
                    );
                }
            }
    };
};

#endif