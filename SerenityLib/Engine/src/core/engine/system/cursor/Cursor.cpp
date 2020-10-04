#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/system/cursor/Cursor.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/resources/Engine_Resources.h>

Cursor::Cursor() {
    bool success = m_SFMLCursor.loadFromSystem(sf::Cursor::Type::Arrow);
}
Cursor::Cursor(const std::string& textureFile) {
    auto texture  = Engine::Resources::getResource<Texture>(textureFile);
    if (!texture.first) {
        texture.second = Engine::Resources::loadTexture(textureFile);
        texture.first  = texture.second.get<Texture>();
    }
    bool success       = loadFromPixels(texture.first, glm::uvec2(0, 0));
}
Cursor::~Cursor() {

}
bool Cursor::internal_load_from_pixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier, bool force) noexcept {
    m_Width           = width;
    m_Height          = height;
    size_t numBytes   = width * height * 4;

    if (force || (m_ColorMultiplier != colorMultiplier) || (m_Pixels.data() != pixels) || (m_Pixels.data() == pixels && m_Pixels.size() != numBytes)) {
        m_ColorMultiplier = colorMultiplier;
        m_Pixels.clear();
        m_Pixels.reserve(numBytes);
        for (size_t i = 0; i < numBytes; i += 4) {
            unsigned char r = static_cast<unsigned char>((float)pixels[i + 0] * colorMultiplier.r);
            unsigned char g = static_cast<unsigned char>((float)pixels[i + 1] * colorMultiplier.g);
            unsigned char b = static_cast<unsigned char>((float)pixels[i + 2] * colorMultiplier.b);
            unsigned char a = static_cast<unsigned char>((float)pixels[i + 3] * colorMultiplier.a);
            m_Pixels.emplace_back(r);
            m_Pixels.emplace_back(g);
            m_Pixels.emplace_back(b);
            m_Pixels.emplace_back(a);
        }
    }
    return m_SFMLCursor.loadFromPixels(m_Pixels.data(), sf::Vector2u(width, height), sf::Vector2u(hotspotX, hotspotY));
}
bool Cursor::internal_rotate(long long startIndex, long long increment1, long long increment2, bool left) noexcept {
    size_t oldWidth  = (size_t)m_Width;
    size_t oldHeight = (size_t)m_Height;
    size_t numBytes  = (size_t)m_Width * (size_t)m_Height * 4;

    std::swap(m_Width, m_Height);
    std::vector<std::uint8_t> oldPixels = m_Pixels;

    m_Pixels.clear();
    m_Pixels.reserve(numBytes);

    long long pixel = startIndex;
    for (size_t i = 0; i < numBytes; i += 4) {
        unsigned char r = static_cast<unsigned char>((float)oldPixels[(pixel * 4) + 0] * m_ColorMultiplier.r);
        unsigned char g = static_cast<unsigned char>((float)oldPixels[(pixel * 4) + 1] * m_ColorMultiplier.g);
        unsigned char b = static_cast<unsigned char>((float)oldPixels[(pixel * 4) + 2] * m_ColorMultiplier.b);
        unsigned char a = static_cast<unsigned char>((float)oldPixels[(pixel * 4) + 3] * m_ColorMultiplier.a);
        m_Pixels.emplace_back(r);
        m_Pixels.emplace_back(g);
        m_Pixels.emplace_back(b);
        m_Pixels.emplace_back(a);
        pixel += increment1;
        if ((left && pixel < 0) || (!left && pixel >= (long long)(oldWidth * oldHeight))) {
            pixel += increment2;
        }
    }
    return m_SFMLCursor.loadFromPixels(m_Pixels.data(), sf::Vector2u(m_Width, m_Height), sf::Vector2u(m_Hotspot.x, m_Hotspot.y));
}
bool Cursor::rotateLeft() noexcept {
    auto oldWidth  = (long long)m_Width;
    auto oldHeight = (long long)m_Height;
    return internal_rotate((oldWidth * oldHeight) - oldWidth, -oldWidth, (oldWidth * oldHeight) + 1, true);
}
bool Cursor::rotateRight() noexcept {
    auto oldWidth  = (long long)m_Width;
    auto oldHeight = (long long)m_Height;
    return internal_rotate(oldWidth - 1, oldWidth, -((oldWidth * oldHeight) + 1), false);
}
void Cursor::setHotspot(unsigned int x, unsigned int y) noexcept {
    m_Hotspot.x = x;
    m_Hotspot.y = y;
    internal_load_from_pixels(m_Pixels.data(), m_Width, m_Height, m_Hotspot.x, m_Hotspot.y, m_ColorMultiplier);
}
bool Cursor::loadFromCurrentData() noexcept {
    return loadFromCurrentData(m_ColorMultiplier);
}
bool Cursor::loadFromCurrentData(const glm::vec4& colorMultiplier) noexcept {
    if (m_Pixels.size() == 0 || m_Width == 0 || m_Height == 0) {
        return false;
    }
    return internal_load_from_pixels(m_Pixels.data(), m_Width, m_Height, m_Hotspot.x, m_Hotspot.y, colorMultiplier);
}

bool Cursor::loadFromPixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    return internal_load_from_pixels(pixels, width, height, hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier) noexcept {
    return internal_load_from_pixels(pixels, width, height, hotspotX, hotspotY, colorMultiplier);
}

bool Cursor::loadFromPixels(Texture* texture, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    assert(texture != nullptr);
    auto pixels = texture->pixels();
    return internal_load_from_pixels(pixels, texture->width(), texture->height(), hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(Texture* texture, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier) noexcept {
    assert(texture != nullptr);
    auto pixels = texture->pixels();
    return internal_load_from_pixels(pixels, texture->width(), texture->height(), hotspotX, hotspotY, colorMultiplier);
}

bool Cursor::loadFromSystem(CursorType cursorType) noexcept {
    return m_SFMLCursor.loadFromSystem((sf::Cursor::Type)cursorType);
}