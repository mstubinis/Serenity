#include <core/engine/networking/Packet.h>
#include <SFML/Network/Packet.hpp>

using namespace std;
using namespace Engine;

Engine::Networking::Packet::Packet(sf::Packet& inSFMLPacket) {
    validate(inSFMLPacket);
}
bool Engine::Networking::Packet::validate(sf::Packet& inSFMLPacket) {
    m_Valid = (inSFMLPacket >> m_PacketType >> m_Timestamp);
    return m_Valid;
}
bool Engine::Networking::Packet::build(sf::Packet& inSFMLPacket) {
    m_Valid = (inSFMLPacket << m_PacketType << m_Timestamp);
    return m_Valid;
}
unsigned int Engine::Networking::Packet::getTypeFromSFPacket(const sf::Packet& inSFMLPacket) {
    sf::Packet hardCopy(inSFMLPacket);
    Engine::Networking::Packet packet(hardCopy);
    return (packet.m_PacketType);
}