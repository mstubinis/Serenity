#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

#include <SFML/Network/Packet.hpp>

using PacketSequence = std::uint16_t;
using PacketBitfield = std::uint32_t;

namespace Engine::Networking {
    class Packet : public sf::Packet {
        public:
            template<typename T> static inline constexpr bool sequence_greater_than(T s1, T s2) noexcept {
                return ((s1 > s2) && (s1 - s2 <= std::numeric_limits<T>().max())) || ((s1 < s2) && (s2 - s1 > std::numeric_limits<T>().max()));
            }
        public:
            bool             m_Valid          = false;
            std::uint32_t    m_PacketType     = 0U;
            std::uint64_t    m_Timestamp      = 0U;
            PacketSequence   m_SequenceNumber = 0U;
            PacketSequence   m_Ack            = 0U;
            PacketBitfield   m_AckBitfield    = 0U;
        public:
            Packet() {}
            Packet(std::uint32_t PacketType) {
                m_PacketType = PacketType;
            }
            Packet(sf::Packet& inSFMLPacket)           = delete;


            Packet& operator=(const Packet& other)     = default;
            Packet(const Packet& other)                = default;
            Packet(Packet&& other) noexcept            = default;
            Packet& operator=(Packet&& other) noexcept = default;
            virtual ~Packet() {}

            void initialize(Engine::Networking::Packet& inPacket) noexcept {
                m_PacketType     = inPacket.m_PacketType;
                m_Timestamp      = inPacket.m_Timestamp;
                m_SequenceNumber = inPacket.m_SequenceNumber;
                m_Ack            = inPacket.m_Ack;
                m_AckBitfield    = inPacket.m_AckBitfield;
                unpack(inPacket);
            }
            void initialize(sf::Packet& inSFMLPacket) noexcept {
                unpack(inSFMLPacket);
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
                using cast          = std::chrono::duration<std::uint64_t>;
                m_Timestamp         = std::chrono::duration_cast<cast>(std::chrono::system_clock::now().time_since_epoch()).count();
                bool initial_build  = (*this << m_PacketType << m_Timestamp << m_SequenceNumber << m_Ack << m_AckBitfield);
                build(*this);
                return sf::Packet::onSend(size);
            }
            virtual void onReceive(const void* data, std::size_t size) override {
                sf::Packet::onReceive(data, size);
                bool initial_unpack = (*this >> m_PacketType >> m_Timestamp >> m_SequenceNumber >> m_Ack >> m_AckBitfield);
                unpack(*this);
            }
    };
};
#endif