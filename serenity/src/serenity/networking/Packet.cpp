
#include <serenity/networking/Packet.h>

sf::Packet& operator <<(sf::Packet& packet, Entity entity) noexcept {
    return packet << entity.id() << entity.versionID() << entity.sceneID();
}
sf::Packet& operator >>(sf::Packet& packet, Entity entity) noexcept {
    auto id      = entity.id();
    auto version = entity.versionID();
    auto scene   = entity.sceneID();
    return packet >> id >> version >> scene;
}

#define BUILD_GLM_VEC_PACKET_DEFINITIONS(NAME) \
    sf::Packet& operator <<(sf::Packet& packet, const glm::##NAME##vec2& data) noexcept { return packet << data.x << data.y; } \
    sf::Packet& operator <<(sf::Packet& packet, const glm::##NAME##vec3& data) noexcept { return packet << data.x << data.y << data.z; } \
    sf::Packet& operator <<(sf::Packet& packet, const glm::##NAME##vec4& data) noexcept { return packet << data.x << data.y << data.z << data.w; } \
    sf::Packet& operator >>(sf::Packet& packet, glm::##NAME##vec2& data) noexcept { return packet >> data.x >> data.y; } \
    sf::Packet& operator >>(sf::Packet& packet, glm::##NAME##vec3& data) noexcept { return packet >> data.x >> data.y >> data.z; } \
    sf::Packet& operator >>(sf::Packet& packet, glm::##NAME##vec4& data) noexcept { return packet >> data.x >> data.y >> data.z >> data.w; } \

#pragma warning( push )
#pragma warning( disable : 4003 )
BUILD_GLM_VEC_PACKET_DEFINITIONS()
#pragma warning( pop )

BUILD_GLM_VEC_PACKET_DEFINITIONS(d)

BUILD_GLM_VEC_PACKET_DEFINITIONS(i64)
BUILD_GLM_VEC_PACKET_DEFINITIONS(i32)
BUILD_GLM_VEC_PACKET_DEFINITIONS(i16)
BUILD_GLM_VEC_PACKET_DEFINITIONS(i8)

BUILD_GLM_VEC_PACKET_DEFINITIONS(u64)
BUILD_GLM_VEC_PACKET_DEFINITIONS(u32)
BUILD_GLM_VEC_PACKET_DEFINITIONS(u16)
BUILD_GLM_VEC_PACKET_DEFINITIONS(u8)