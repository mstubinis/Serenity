#pragma once
#ifndef ENGINE_NETWORKING_PACKET_H
#define ENGINE_NETWORKING_PACKET_H

namespace sf {
    class Packet;
};
namespace Engine::Networking {
    class IPacket {
        public:
            virtual bool validate(sf::Packet& sfmlPacket) = 0;
            virtual bool build(sf::Packet& sfmlPacket) = 0;
    };
    class Packet : public IPacket {
        public:
            bool         m_Valid      = false;
            unsigned int m_PacketType = 0;
        public:
            Packet();
            Packet(sf::Packet& sfPacket);
            virtual ~Packet();

            Packet& operator=(const Packet& other);
            Packet(const Packet& other);
            Packet(Packet&& other) noexcept;
            Packet& operator=(Packet&& other) noexcept;

            virtual bool validate(sf::Packet& sfmlPacket) override;
            virtual bool build(sf::Packet& sfmlPacket) override;
            virtual void print() {}

            static unsigned int getTypeFromSFPacket(const sf::Packet& sfmlPacket);
    };
};
#endif