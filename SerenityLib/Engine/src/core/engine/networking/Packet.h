#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

namespace sf {
    class Packet;
};

namespace Engine::Networking {
    class Packet {
        public:
            bool                      m_Valid       = false;
            unsigned int              m_PacketType  = 0;
            std::uint64_t             m_Timestamp;
        public:
            Packet() {}
            Packet(sf::Packet& inSFMLPacket);
            //virtual ~Packet() {}
            ~Packet() {}

            Packet& operator=(const Packet& other)     = default;
            Packet(const Packet& other)                = default;
            Packet(Packet&& other) noexcept            = default;
            Packet& operator=(Packet&& other) noexcept = default;

            virtual bool validate(sf::Packet& inSFMLPacket);
            virtual bool build(sf::Packet& inSFMLPacket);
            //virtual void print() {}

            static unsigned int getTypeFromSFPacket(const sf::Packet& inSFMLPacket);
    };
};
#endif