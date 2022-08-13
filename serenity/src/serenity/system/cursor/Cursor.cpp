
#include <serenity/system/cursor/Cursor.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/Engine_Resources.h>

class Cursor::Impl {
public:
    //pixels must be an array of width by height pixels in 32 - bit RGBA format.If not, this will cause undefined behavior.
    static bool internal_load_from_pixels(Cursor& cursor, const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
        const uint32_t colorsPerPixel = 4;
        cursor.m_Width         = width;
        cursor.m_Height        = height;
        const size_t numBytes  = width * height * colorsPerPixel;
        cursor.m_PixelsColored = cursor.m_Pixels;

        cursor.m_ColorMultiplier = colorMultiplier;
        cursor.m_PixelsColored.resize(numBytes);
        using idxType = glm::vec4::length_type;
        for (size_t i = 0; i < numBytes; i += colorsPerPixel) {
            for (size_t j = 0; j < colorsPerPixel; ++j) {
                cursor.m_PixelsColored[i + j] = uint8_t(float(pixels[i + j] * colorMultiplier[idxType(j)]));
            }
        }
        Engine::priv::TextureLoader::MirrorPixelsVertically(cursor.m_PixelsColored.data(), width, height, colorsPerPixel);

        return cursor.m_SFMLCursor.loadFromPixels(cursor.m_PixelsColored.data(), sf::Vector2u(width, height), sf::Vector2u(hotspotX, hotspotY));
    }

    static bool internal_rotate(Cursor& cursor, int64_t startIndex, int64_t increment1, int64_t increment2, bool left) noexcept {
        const size_t oldWidth  = size_t(cursor.m_Width);
        const size_t oldHeight = size_t(cursor.m_Height);
        const size_t numBytes  = size_t(cursor.m_Width) * size_t(cursor.m_Height) * size_t(4);

        //rotate the pixels
        std::swap(cursor.m_Width, cursor.m_Height);
        int64_t pixel = startIndex;

        auto newPixels = Engine::create_and_reserve<std::vector<uint8_t>>(numBytes);
        for (size_t i = 0; i < numBytes; i += 4) {
            for (size_t j = 0; j < 4; ++j) {
                newPixels.emplace_back(uint8_t(float(cursor.m_Pixels[(pixel * 4) + j])));
            }
            pixel += increment1;
            if ((left && pixel < 0) || (!left && pixel >= (long long)(oldWidth * oldHeight))) {
                pixel += increment2;
            }
        }
        cursor.m_Pixels = newPixels;
        //color the pixels
        cursor.m_PixelsColored.clear();
        cursor.m_PixelsColored.reserve(numBytes);
        using idxType = glm::vec4::length_type;
        for (size_t i = 0; i < cursor.m_Pixels.size(); i += 4) {
            for (size_t j = 0; j < 4; ++j) {
                cursor.m_PixelsColored.emplace_back(uint8_t(float(cursor.m_Pixels[i + j] * cursor.m_ColorMultiplier[idxType(j)])));
            }
        }
        return cursor.m_SFMLCursor.loadFromPixels(cursor.m_PixelsColored.data(), sf::Vector2u(cursor.m_Width, cursor.m_Height), sf::Vector2u(cursor.m_Hotspot.x, cursor.m_Hotspot.y));
    }
};


Cursor::Cursor() {
    bool success = m_SFMLCursor.loadFromSystem(sf::Cursor::Type::Arrow);
}
Cursor::Cursor(std::string_view textureFile) {
    auto texture = Engine::Resources::getResource<Texture>(textureFile);
    if (!texture.m_Resource) {
        texture.m_Handle   = Engine::Resources::loadTexture(textureFile);
        texture.m_Resource = texture.m_Handle.get<Texture>();
    }
    bool success = loadFromPixels(texture.m_Resource, glm::uvec2{0, 0});
}
Cursor::~Cursor() {

}

bool Cursor::rotateLeft() noexcept {
    const auto oldWidth  = int64_t(m_Width);
    const auto oldHeight = int64_t(m_Height);
    return Impl::internal_rotate(*this, (oldWidth * oldHeight) - oldWidth, -oldWidth, (oldWidth * oldHeight) + 1, true);
}
bool Cursor::rotateRight() noexcept {
    const auto oldWidth  = int64_t(m_Width);
    const auto oldHeight = int64_t(m_Height);
    return Impl::internal_rotate(*this, oldWidth - 1, oldWidth, -((oldWidth * oldHeight) + 1), false);
}
void Cursor::setHotspot(uint32_t x, uint32_t y) noexcept {
    m_Hotspot.x = x;
    m_Hotspot.y = y;
    Impl::internal_load_from_pixels(*this, m_Pixels.data(), m_Width, m_Height, m_Hotspot.x, m_Hotspot.y, m_ColorMultiplier);
}
bool Cursor::loadFromCurrentData() noexcept {
    return loadFromCurrentData(m_ColorMultiplier);
}
bool Cursor::loadFromCurrentData(const glm::vec4& colorMultiplier) noexcept {
    if (m_Pixels.size() == 0 || m_Width == 0 || m_Height == 0) {
        return false;
    }
    return Impl::internal_load_from_pixels(*this, m_Pixels.data(), m_Width, m_Height, m_Hotspot.x, m_Hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    m_Pixels = std::vector<uint8_t>(pixels, pixels + size_t(width) * size_t(height) * size_t(4));
    return Impl::internal_load_from_pixels(*this, pixels, width, height, hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
    m_Pixels = std::vector<uint8_t>(pixels, pixels + size_t(width) * size_t(height) * size_t(4));
    return Impl::internal_load_from_pixels(*this, pixels, width, height, hotspotX, hotspotY, colorMultiplier);
}
bool Cursor::loadFromPixels(Texture* texture, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier) noexcept {
    ASSERT(texture, __FUNCTION__ << "(): texture was nullptr!");
    const auto pixels = texture->pixels();
    m_Pixels = std::vector<uint8_t>(pixels, pixels + size_t(texture->width()) * size_t(texture->height()) * size_t(4));
    return Impl::internal_load_from_pixels(*this, pixels, texture->width(), texture->height(), hotspot.x, hotspot.y, colorMultiplier);
}
bool Cursor::loadFromPixels(Texture* texture, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept {
    ASSERT(texture, __FUNCTION__ << "(): texture was nullptr!");
    const auto pixels = texture->pixels();
    m_Pixels = std::vector<uint8_t>(pixels, pixels + size_t(texture->width()) * size_t(texture->height()) * size_t(4));
    return Impl::internal_load_from_pixels(*this, pixels, texture->width(), texture->height(), hotspotX, hotspotY, colorMultiplier);
}
bool Cursor::loadFromSystem(CursorType cursorType) noexcept {
    return m_SFMLCursor.loadFromSystem(static_cast<sf::Cursor::Type>(cursorType));
}