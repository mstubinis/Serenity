#pragma once
#ifndef ENGINE_SYSTEM_CURSOR_H
#define ENGINE_SYSTEM_CURSOR_H

class  Texture;

#include <SFML/Window/Cursor.hpp>
#include <serenity/system/TypeDefs.h>
#include <serenity/dependencies/glm.h>
#include <vector>

enum class CursorType : uint8_t {
    Arrow = sf::Cursor::Type::Arrow, //Arrow cursor (default)
    ArrowWait,                       //Busy arrow cursor. Not allowed on Linux or Mac OS X
    Wait,                            //Busy cursor. Not allowed on Mac OS X
    Text,                            //I-beam, cursor when hovering over a field allowing text entry.
    Hand,                            //Pointing hand cursor.
    SizeHorizontal,                  //Horizontal double arrow cursor.
    SizeVertical,                    //Vertical double arrow cursor.
    SizeTopLeftBottomRight,          //Double arrow cursor going from top-left to bottom-right. Not allowed on Linux
    SizeBottomLeftTopRight,          //Double arrow cursor going from bottom-left to top-right. Not allowed on Linux
    SizeAll,                         //Combination of SizeHorizontal and SizeVertical. Not allowed on Mac OS X
    Cross,                           //Crosshair cursor.
    Help,                            //Help cursor.
    NotAllowed,                      //Action not allowed cursor.
};

class Cursor {
    private:
        sf::Cursor                m_SFMLCursor;
        std::vector<uint8_t>      m_Pixels;
        std::vector<uint8_t>      m_PixelsColored;
        uint32_t                  m_Width           = 0;
        uint32_t                  m_Height          = 0;
        glm::vec4                 m_ColorMultiplier = glm::vec4(1.0f);
        glm::uvec2                m_Hotspot         = glm::uvec2(0, 0);

        //pixels must be an array of width by height pixels in 32 - bit RGBA format.If not, this will cause undefined behavior.
        bool internal_load_from_pixels(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier) noexcept;

        bool internal_rotate(long long startIndex, long long increment, long long increment2, bool left) noexcept;
    protected:

    public:
        Cursor();
        Cursor(std::string_view textureFile);
        virtual ~Cursor();

        virtual bool rotateLeft() noexcept;
        virtual bool rotateRight() noexcept;

        virtual bool loadFromCurrentData() noexcept;
        virtual bool loadFromCurrentData(const glm::vec4& colorMultiplier) noexcept;

        virtual bool loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;
        virtual bool loadFromPixels(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;

        virtual bool loadFromPixels(Texture* texture, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;
        virtual bool loadFromPixels(Texture* texture, uint32_t hotspotX, uint32_t hotspotY, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;

        virtual bool loadFromSystem(CursorType cursorType) noexcept;

        virtual void setHotspot(uint32_t x, uint32_t y) noexcept;

        [[nodiscard]] inline constexpr const glm::vec4& getColor() const noexcept { return m_ColorMultiplier; }
        [[nodiscard]] inline constexpr const sf::Cursor& getSFMLCursor() const noexcept { return m_SFMLCursor; }
        [[nodiscard]] inline constexpr uint32_t getWidth() const noexcept { return m_Width; }
        [[nodiscard]] inline constexpr uint32_t getHeight() const noexcept { return m_Height; }
        [[nodiscard]] inline constexpr const std::vector<uint8_t>& getPixels() const noexcept { return m_Pixels; }
};

#endif