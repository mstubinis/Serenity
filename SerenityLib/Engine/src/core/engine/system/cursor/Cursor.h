#pragma once
#ifndef ENGINE_SYSTEM_CURSOR_H
#define ENGINE_SYSTEM_CURSOR_H

class  Texture;

#include <SFML/Window/Cursor.hpp>

enum class CursorType : unsigned short {
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
        sf::Cursor                               m_SFMLCursor;
        std::vector<std::uint8_t>                m_Pixels;
        unsigned int                             m_Width           = 0;
        unsigned int                             m_Height          = 0;
        glm::vec4                                m_ColorMultiplier = glm::vec4(1.0f);
        glm::uvec2                               m_Hotspot         = glm::uvec2(0, 0);

        //pixels must be an array of width by height pixels in 32 - bit RGBA format.If not, this will cause undefined behavior.
        bool internal_load_from_pixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier, bool force = false) noexcept;

        bool internal_rotate(long long startIndex, long long increment, long long increment2, bool left) noexcept;
    protected:

    public:
        Cursor();
        Cursor(const std::string& textureFile);
        virtual ~Cursor();

        virtual bool rotateLeft() noexcept;
        virtual bool rotateRight() noexcept;

        virtual bool loadFromCurrentData() noexcept;
        virtual bool loadFromCurrentData(const glm::vec4& colorMultiplier) noexcept;

        virtual bool loadFromPixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;
        virtual bool loadFromPixels(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;

        virtual bool loadFromPixels(Texture* texture, const glm::uvec2& hotspot, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;
        virtual bool loadFromPixels(Texture* texture, unsigned int hotspotX, unsigned int hotspotY, const glm::vec4& colorMultiplier = glm::vec4(1.0f)) noexcept;

        virtual bool loadFromSystem(CursorType cursorType) noexcept;

        virtual void setHotspot(unsigned int x, unsigned int y) noexcept;

        inline CONSTEXPR const sf::Cursor& getSFMLCursor() const noexcept { return m_SFMLCursor; }
        inline CONSTEXPR unsigned int getWidth() const noexcept { return m_Width; }
        inline CONSTEXPR unsigned int getHeight() const noexcept { return m_Height; }
        inline CONSTEXPR const std::vector<std::uint8_t>& getPixels() const noexcept { return m_Pixels; }
};

#endif