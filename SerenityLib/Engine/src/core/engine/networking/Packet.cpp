#include <core/engine/networking/Packet.h>
#include <SFML/Network/Packet.hpp>

using namespace std;
using namespace Engine;

Engine::Networking::Packet::Packet() {

}
Engine::Networking::Packet::Packet(sf::Packet& sfPacket) {
    validate(sfPacket);
}
Engine::Networking::Packet::~Packet() {

}
Engine::Networking::Packet& Engine::Networking::Packet::operator=(const Packet& other) {
    if (&other != this) {
        m_PacketType = other.m_PacketType;
        m_Valid      = other.m_Valid;
        return *this;
    }
}
Engine::Networking::Packet::Packet(const Packet& other) {
    m_PacketType = other.m_PacketType;
    m_Valid      = other.m_Valid;
}
Engine::Networking::Packet::Packet(Packet&& other) noexcept {
    m_PacketType = std::move(other.m_PacketType);
    m_Valid      = std::move(other.m_Valid);
}
Engine::Networking::Packet& Engine::Networking::Packet::operator=(Packet&& other) noexcept {
    if (&other != this) {
        m_PacketType = std::move(other.m_PacketType);
        m_Valid      = std::move(other.m_Valid);
    }
    return *this;
}

bool Engine::Networking::Packet::validate(sf::Packet& sfPacket) {
    m_Valid = (sfPacket >> m_PacketType);
    return m_Valid;
}
bool Engine::Networking::Packet::build(sf::Packet& sfPacket) {
    m_Valid = (sfPacket << m_PacketType);
    return m_Valid;
}
unsigned int Engine::Networking::Packet::getTypeFromSFPacket(const sf::Packet& sfPacket) {
    sf::Packet hardCopy(sfPacket);
    Engine::Networking::Packet packet(hardCopy);
    return (packet.m_PacketType);
}