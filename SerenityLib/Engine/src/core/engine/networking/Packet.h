#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

#include <SFML/Network/Packet.hpp>

using PacketSequence  = uint16_t;
using PacketBitfield  = uint32_t;
using PacketTimestamp = uint32_t;

namespace Engine::Networking {
    class Server;
    class ServerClient;
};


//glm vector floats
sf::Packet& operator <<(sf::Packet& packet, const glm::vec2& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::vec3& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::vec4& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::vec2& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::vec3& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::vec4& data) noexcept;
//glm vector ints
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec2& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec3& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec4& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::ivec2& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::ivec3& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::ivec4& data) noexcept;
//glm vector uints
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec2& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec3& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec4& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::uvec2& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::uvec3& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::uvec4& data) noexcept;
//glm vector doubles
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec2& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec3& data) noexcept;
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec4& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::dvec2& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::dvec3& data) noexcept;
sf::Packet& operator >>(sf::Packet& packet, glm::dvec4& data) noexcept;

template<typename T, size_t size>
inline sf::Packet& operator <<(sf::Packet& packet, const std::array<T, size>& data) noexcept {
    for (size_t i = 0; i < size - 1; ++i) {
        packet << data[i];
    }
    return packet << data[size - 1];
}
template<typename T, size_t size>
inline sf::Packet& operator >>(sf::Packet& packet, std::array<T, size>& data) noexcept {
    for (size_t i = 0; i < size - 1; ++i) {
        packet >> data[i];
    }
    return packet >> data[size - 1];
}

namespace Engine::Networking {
    class Packet : public sf::Packet {
        using SendFP = std::function<void(Engine::Networking::Packet* packet)>;
        public:
            template<typename T> static inline constexpr bool sequence_greater_than(T s1, T s2) noexcept {
                return ((s1 > s2) && (s1 - s2 <= std::numeric_limits<T>().max())) || ((s1 < s2) && (s2 - s1 > std::numeric_limits<T>().max()));
            }
        private:
            SendFP m_OnSendFunction = [](Engine::Networking::Packet* packet) {};
        public:
            uint32_t           m_PacketType     = 0U;
            PacketTimestamp    m_Timestamp      = 0U;
            PacketBitfield     m_AckBitfield    = 0U;
            PacketSequence     m_SequenceNumber = 0U;
            PacketSequence     m_Ack            = 0U;
            bool               m_Valid          = false;
        public:
            Packet() = default;
            Packet(uint32_t PacketType)
                : m_PacketType { PacketType }
            {}
            Packet(sf::Packet& inSFMLPacket)           = delete;

            Packet& operator=(const Packet& other)     = default;
            Packet(const Packet& other)                = default;
            Packet(Packet&& other) noexcept            = default;
            Packet& operator=(Packet&& other) noexcept = default;
            virtual ~Packet() {}

            void initialize(Engine::Networking::Packet& inPacket) noexcept {        
                m_PacketType     = inPacket.m_PacketType;
                m_Timestamp      = inPacket.m_Timestamp;
                m_AckBitfield    = inPacket.m_AckBitfield;
                m_SequenceNumber = inPacket.m_SequenceNumber;
                m_Ack            = inPacket.m_Ack;
                if (!inPacket.endOfPacket()) {
                    unpack(inPacket);
                }
            }
            void initialize(sf::Packet& inSFMLPacket) noexcept {
                if (!inSFMLPacket.endOfPacket()){
                    unpack(inSFMLPacket);
                }
            }

            inline void setOnSendFunction(SendFP&& function) noexcept {
                m_OnSendFunction = std::move(function);
            }

            virtual Engine::Networking::Packet* clone() {
                return NEW Engine::Networking::Packet(*this);
            }
            virtual bool unpack(sf::Packet& inSFMLPacket) {
                m_Valid = true;
                return true;
            }
            virtual bool build(sf::Packet& inSFMLPacket) {
                return true;
            }
            virtual const void* onSend(std::size_t& size) override {
                clear();
                m_OnSendFunction(this);
                using cast          = std::chrono::milliseconds;
                m_Timestamp         = (PacketTimestamp)std::chrono::duration_cast<cast>(std::chrono::system_clock::now().time_since_epoch()).count();
                bool initial_build  = (*this 
                    << m_PacketType 
                    << m_Timestamp 
                    << m_AckBitfield
                    << m_SequenceNumber
                    << m_Ack
                );
                build(*this);
                return sf::Packet::onSend(size);
            }
            virtual void onReceive(const void* data, std::size_t size) override {
                sf::Packet::onReceive(data, size);
                if (!endOfPacket()) {
                    bool initial_unpack = (*this 
                        >> m_PacketType
                        >> m_Timestamp
                        >> m_AckBitfield
                        >> m_SequenceNumber
                        >> m_Ack
                    );
                }
                if (!endOfPacket()) {
                    unpack(*this);
                }
            }
    };
};

#endif