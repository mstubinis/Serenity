#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

#include <SFML/Network/Packet.hpp>

namespace Engine::Networking {
    class Packet : public sf::Packet {
        private:
            bool internal_validate() { 
                m_Valid = true; 
                return true; 
            }
            bool internal_build() { 
                m_Valid = true; 
                return true; 
            }
        public:
            bool                      m_Valid       = false;
            unsigned int              m_PacketType  = 0U;
            std::uint64_t             m_Timestamp   = 0U;
        public:
            Packet() {}
            Packet(unsigned int PacketType) {
                m_PacketType = PacketType;
            }
            Packet(sf::Packet& inSFMLPacket, bool doValidate = false) : sf::Packet(inSFMLPacket){
                bool initial_result = (*this >> m_PacketType >> m_Timestamp);
                if (doValidate) {
                    validate(*this);
                }
            }
            Packet& operator=(const Packet& other)     = default;
            Packet(const Packet& other)                = default;
            Packet(Packet&& other) noexcept            = default;
            Packet& operator=(Packet&& other) noexcept = default;
            virtual ~Packet() {}

            void initialize(Engine::Networking::Packet& inPacket) {
                m_PacketType = inPacket.m_PacketType;
                m_Timestamp  = inPacket.m_Timestamp;
                validate(inPacket);
            }
            void initialize(sf::Packet& inSFMLPacket) {
                validate(inSFMLPacket);
            }

            virtual bool validate(sf::Packet& inSFMLPacket) {
                m_Valid = true; 
                return true; 
            }
            virtual bool build(sf::Packet& inSFMLPacket) {
                return true; 
            }
            virtual const void* onSend(std::size_t& size) override {
                using cast         = std::chrono::duration<std::uint64_t>;
                m_Timestamp        = std::chrono::duration_cast<cast>(std::chrono::system_clock::now().time_since_epoch()).count();
                bool initial_build = (*this << m_PacketType << m_Timestamp);
                build(*this);
                return sf::Packet::onSend(size);
            }
            virtual void onReceive(const void* data, std::size_t size) override {
                sf::Packet::onReceive(data, size);
                bool initial_result = (*this >> m_PacketType >> m_Timestamp);
                validate(*this);
            }
    };
};
#endif