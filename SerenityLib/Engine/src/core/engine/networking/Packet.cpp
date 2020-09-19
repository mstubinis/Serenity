#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/Packet.h>

//glm vector floats
sf::Packet& operator <<(sf::Packet& packet, const glm::vec2& data) { return packet << data.x << data.y; }
sf::Packet& operator <<(sf::Packet& packet, const glm::vec3& data) { return packet << data.x << data.y << data.z; }
sf::Packet& operator <<(sf::Packet& packet, const glm::vec4& data) { return packet << data.x << data.y << data.z << data.w; }
sf::Packet& operator >>(sf::Packet& packet, glm::vec2& data) { return packet >> data.x >> data.y; }
sf::Packet& operator >>(sf::Packet& packet, glm::vec3& data) { return packet >> data.x >> data.y >> data.z; }
sf::Packet& operator >>(sf::Packet& packet, glm::vec4& data) { return packet >> data.x >> data.y >> data.z >> data.w; }
//glm vector ints
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec2& data) { return packet << data.x << data.y; }
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec3& data) { return packet << data.x << data.y << data.z; }
sf::Packet& operator <<(sf::Packet& packet, const glm::ivec4& data) { return packet << data.x << data.y << data.z << data.w; }
sf::Packet& operator >>(sf::Packet& packet, glm::ivec2& data) { return packet >> data.x >> data.y; }
sf::Packet& operator >>(sf::Packet& packet, glm::ivec3& data) { return packet >> data.x >> data.y >> data.z; }
sf::Packet& operator >>(sf::Packet& packet, glm::ivec4& data) { return packet >> data.x >> data.y >> data.z >> data.w; }
//glm vector uints
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec2& data) { return packet << data.x << data.y; }
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec3& data) { return packet << data.x << data.y << data.z; }
sf::Packet& operator <<(sf::Packet& packet, const glm::uvec4& data) { return packet << data.x << data.y << data.z << data.w; }
sf::Packet& operator >>(sf::Packet& packet, glm::uvec2& data) { return packet >> data.x >> data.y; }
sf::Packet& operator >>(sf::Packet& packet, glm::uvec3& data) { return packet >> data.x >> data.y >> data.z; }
sf::Packet& operator >>(sf::Packet& packet, glm::uvec4& data) { return packet >> data.x >> data.y >> data.z >> data.w; }
//glm vector doubles
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec2& data) { return packet << data.x << data.y; }
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec3& data) { return packet << data.x << data.y << data.z; }
sf::Packet& operator <<(sf::Packet& packet, const glm::dvec4& data) { return packet << data.x << data.y << data.z << data.w; }
sf::Packet& operator >>(sf::Packet& packet, glm::dvec2& data) { return packet >> data.x >> data.y; }
sf::Packet& operator >>(sf::Packet& packet, glm::dvec3& data) { return packet >> data.x >> data.y >> data.z; }
sf::Packet& operator >>(sf::Packet& packet, glm::dvec4& data) { return packet >> data.x >> data.y >> data.z >> data.w; }
