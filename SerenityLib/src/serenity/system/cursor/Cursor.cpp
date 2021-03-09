
#include <serenity/system/cursor/Cursor.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/Engine_Resources.h>

Cursor::Cursor() {
    bool success = m_SFMLCursor.loadFromSystem(sf::Cursor::Type::Arrow);
}
Cursor::Cursor(std::string_view textureFile) {
    auto texture  = Engine::Resources::getResource<Texture>(textureFile);
    if (!texture.m_Resource) {
        texture.m_Handle   = Engine::Resources::loadTexture(textureFile);
        texture.m_Resource = texture.m_Handle.get<Texture>();
    }
    bool success  = loadFromPixels(texture.m_Resource, glm::uvec2(0, 0));
}
Cursor::~Cursor() {

}
bool Cursor::internal_load_from_pixels(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
    m_Width           = width;
    m_Height          = height;
    size_t numBytes   = width * height * 4;
    m_PixelsColored   = m_Pixels;

    m_ColorMultiplier = colorMultiplier;
    m_PixelsColored.resize(numBytes);
    for (size_t i = 0; i < numBytes; i += 4) {
        uint8_t r = static_cast<uint8_t>((float)pixels[i + 0] * colorMultiplier.r);
        uint8_t g = static_cast<uint8_t>((float)pixels[i + 1] * colorMultiplier.g);
        uint8_t b = static_cast<uint8_t>((float)pixels[i + 2] * colorMultiplier.b);
        uint8_t a = static_cast<uint8_t>((float)pixels[i + 3] * colorMultiplier.a);
        m_PixelsColored[i + 0] = r;
        m_PixelsColored[i + 1] = g;
        m_PixelsColored[i + 2] = b;
        m_PixelsColored[i + 3] = a;
    }
    bool result = m_SFMLCursor.loadFromPixels(m_PixelsColored.data(), sf::Vector2u(width, height), sf::Vector2u(hotspotX, hotspotY));
    return result;
}
bool Cursor::internal_rotate(long long startIndex, long long increment1, long long increment2, bool left) noexcept {
    size_t oldWidth  = (size_t)m_Width;
    size_t oldHeight = (size_t)m_Height;
    size_t numBytes  = (size_t)m_Width * (size_t)m_Height * 4;

    //rotate the pixels
    std::swap(m_Width, m_Height);
    long long pixel = startIndex;

    auto newPixels = Engine::create_and_reserve<std::vector<uint8_t>>(numBytes);
    for (size_t i = 0; i < numBytes; i += 4) {
        uint8_t r = static_cast<uint8_t>((float)m_Pixels[(pixel * 4) + 0]);
        uint8_t g = static_cast<uint8_t>((float)m_Pixels[(pixel * 4) + 1]);
        uint8_t b = static_cast<uint8_t>((float)m_Pixels[(pixel * 4) + 2]);
        uint8_t a = static_cast<uint8_t>((float)m_Pixels[(pixel * 4) + 3]);
        newPixels.emplace_back(r);
        newPixels.emplace_back(g);
        newPixels.emplace_back(b);
        newPixels.emplace_back(a);
        pixel += increment1;
        if ((left && pixel < 0) || (!left && pixel >= (long long)(oldWidth * oldHeight))) {
            pixel += increment2;
        }
    }
    m_Pixels = newPixels;
    //color the pixels
    m_PixelsColored.clear();
    m_PixelsColored.reserve(numBytes);
    for (size_t i = 0; i < m_Pixels.size(); i += 4) {
        uint8_t r = static_cast<uint8_t>((float)m_Pixels[i + 0] * m_ColorMultiplier.r);
        uint8_t g = static_cast<uint8_t>((float)m_Pixels[i + 1] * m_ColorMultiplier.g);
        uint8_t b = static_cast<uint8_t>((float)m_Pixels[i + 2] * m_ColorMultiplier.b);
        uint8_t a = static_cast<uint8_t>((float)m_Pixels[i + 3] * m_ColorMultiplier.a);
        m_PixelsColored.emplace_back(r);
        m_PixelsColored.emplace_back(g);
        m_PixelsColored.emplace_back(b);
        m_PixelsColored.emplace_back(a);
    }
    bool result = m_SFMLCursor.loadFromPixels(m_PixelsColored.data(), sf::Vector2u(m_Width, m_Height), sf::Vector2u(m_Hotspot.x, m_Hotspot.y));
    return result;
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
void Cursor::setHotspot(uint32_t x, uint32_t y) noexcept {
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

bool Cursor::loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    m_Pixels = std::vector<uint8_t>(pixels, pixels + (width * height * 4));
    return internal_load_from_pixels(pixels, width, height, hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
    m_Pixels = std::vector<uint8_t>(pixels, pixels + (width * height * 4));
    return internal_load_from_pixels(pixels, width, height, hotspotX, hotspotY, colorMultiplier);
}

bool Cursor::loadFromPixels(Texture* texture, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    ASSERT(texture, __FUNCTION__ << "(): texture was nullptr!");
    auto pixels = texture->pixels();
    m_Pixels = std::vector<uint8_t>(pixels, pixels + (size_t)(texture->width() * texture->height() * 4));
    return internal_load_from_pixels(pixels, texture->width(), texture->height(), hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(Texture* texture, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
    ASSERT(texture, __FUNCTION__ << "(): texture was nullptr!");
    auto pixels = texture->pixels();
    m_Pixels = std::vector<uint8_t>(pixels, pixels + (size_t)(texture->width() * texture->height() * 4));
    return internal_load_from_pixels(pixels, texture->width(), texture->height(), hotspotX, hotspotY, colorMultiplier);
}

bool Cursor::loadFromSystem(CursorType cursorType) noexcept {
    return m_SFMLCursor.loadFromSystem((sf::Cursor::Type)cursorType);
}