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
using PacketTimestamp = int64_t;

namespace Engine {
    class quat32;
    class quat64;
}
namespace Engine::Networking {
    class Server;
    class ServerClient;
};

sf::Packet& operator <<(sf::Packet&, Entity) noexcept;
sf::Packet& operator >>(sf::Packet&, Entity) noexcept;


sf::Packet& operator <<(sf::Packet&, const Engine::quat32&) noexcept;
sf::Packet& operator >>(sf::Packet&, Engine::quat32&) noexcept;

sf::Packet& operator <<(sf::Packet&, const Engine::quat64&) noexcept;
sf::Packet& operator >>(sf::Packet&, Engine::quat64&) noexcept;

#define BUILD_GLM_VEC_PACKET_DECLS(NAME) \
    sf::Packet& operator <<(sf::Packet&, const glm::##NAME##vec2&) noexcept; \
    sf::Packet& operator <<(sf::Packet&, const glm::##NAME##vec3&) noexcept; \
    sf::Packet& operator <<(sf::Packet&, const glm::##NAME##vec4&) noexcept; \
    sf::Packet& operator >>(sf::Packet&, glm::##NAME##vec2&) noexcept; \
    sf::Packet& operator >>(sf::Packet&, glm::##NAME##vec3&) noexcept; \
    sf::Packet& operator >>(sf::Packet&, glm::##NAME##vec4&) noexcept; \

#pragma warning( push )
#pragma warning( disable : 4003 )
BUILD_GLM_VEC_PACKET_DECLS()    //glm vector floats
#pragma warning( pop )

BUILD_GLM_VEC_PACKET_DECLS(d)   //glm vector doubles

BUILD_GLM_VEC_PACKET_DECLS(i64) //glm vector int64s
BUILD_GLM_VEC_PACKET_DECLS(i32) //glm vector int32s
BUILD_GLM_VEC_PACKET_DECLS(i16) //glm vector int16s
BUILD_GLM_VEC_PACKET_DECLS(i8)  //glm vector int8s

BUILD_GLM_VEC_PACKET_DECLS(u64) //glm vector uint64s
BUILD_GLM_VEC_PACKET_DECLS(u32) //glm vector uint32s
BUILD_GLM_VEC_PACKET_DECLS(u16) //glm vector uint16s
BUILD_GLM_VEC_PACKET_DECLS(u8)  //glm vector uint8s

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
        vector.push_back(std::move(item));
    }
    return packet;
}


template<class T>
inline sf::Packet& operator <<(sf::Packet& packet, const std::optional<T>& optional_) noexcept {
    bool hasValue = optional_.has_value();
    T val = hasValue ? optional_.value() : T{};
    packet << hasValue;
    packet << val;
    return packet;
}
template<class T>
inline sf::Packet& operator >>(sf::Packet& packet, std::optional<T>& optional_) noexcept {
    T item;
    bool isPresent;
    packet >> isPresent;
    packet >> item;
    if (isPresent) {
        optional_.emplace(std::move(item));
    } else {
        optional_ = std::optional<T>{};
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
            PacketTimestamp    m_Timestamp      = 0;
            uint32_t           m_PacketType     = 0;
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