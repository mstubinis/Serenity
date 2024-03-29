#include <serenity/resources/font/Font.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/texture/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <filesystem>

#include <ft2build.h>
#include <freetype/freetype.h> //can be replaced with #include FT_FREETYPE_H

#include <SFML/Graphics/Image.hpp>

#include <sstream>

namespace {
    auto& getFromRowCol(auto& container, size_t row, size_t col, size_t colSize) {
        return container[(row * colSize) + col];
    }
}

Font::Font(const std::string& filename, int height, int width, float line_height) 
    : Resource{ ResourceType::Font, filename }
    , m_LineHeight{ line_height }
{
    init(filename, height, width);
}
Font::Font(Font&& other) noexcept 
    : Resource(std::move(other))
    , m_CharGlyphs  { std::move(other.m_CharGlyphs) }
    , m_FontTexture { std::exchange(other.m_FontTexture, Handle{}) }
    , m_MaxHeight   { std::move(other.m_MaxHeight) }
    , m_LineHeight  { std::move(other.m_LineHeight) }
{}
Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        m_CharGlyphs  = std::move(other.m_CharGlyphs);
        m_FontTexture = std::exchange(other.m_FontTexture, Handle{});
        m_MaxHeight   = std::move(other.m_MaxHeight);
        m_LineHeight  = std::move(other.m_LineHeight);
    }
    return *this;
}
void Font::init(const std::string& filename, int height, int width) {
    const std::string extension = std::filesystem::path(filename).extension().string();
    /*
    TODO:
        CID-keyed Type 1 fonts
        SFNT-based bitmap fonts, including color Emoji
        Type 42 fonts (limited support)
    */
    if (extension == ".fnt") { //Windows FNT fonts
        init_simple(filename, height, width);
    } else if (extension == ".ttf" || extension == ".ttc") { //TrueType fonts (TTF) and TrueType collections (TTC)
        init_freetype(filename, height, width);
    } else if (extension == ".cff") { //CFF fonts
        init_freetype(filename, height, width);
    } else if (extension == ".pcf") { //X11 PCF fonts
        init_freetype(filename, height, width);
    } else if (extension == ".woff") { //WOFF fonts
        init_freetype(filename, height, width);
    } else if (extension == ".otf" || extension == ".otc") { //OpenType fonts (OTF, both TrueType and CFF variants) and OpenType collections (OTC)
        init_freetype(filename, height, width);
    } else if (extension == ".pfa" || extension == ".pfb") { //Type 1 fonts (PFA and PFB)
        init_freetype(filename, height, width);
    } else if (extension == ".bdf") { //BDF fonts (including anti-aliased ones)
        init_freetype(filename, height, width);
    } else if (extension == ".pfr") { //PFR fonts
        init_freetype(filename, height, width);
    }
}

void Font::init_simple(const std::string& filename, int height, int width) {
    std::string rawname    = filename;
    const size_t lastindex = filename.find_last_of(".");
    if (lastindex != std::string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }

    m_FontTexture = Engine::Resources::addResource<Texture>(rawname, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
    auto& texture = *m_FontTexture.get<Texture>();

    float min_y_offset  = std::numeric_limits<float>().max();
    float max_y_offset  = std::numeric_limits<float>().min();
    float textureHeight = float(texture.height());
    float textureWidth  = float(texture.width());

    boost::iostreams::stream<boost::iostreams::mapped_file_source> source(filename);
    for (std::string line; std::getline(source, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            auto charGlyph = CharGlyph();
            std::string token;
            std::istringstream stream(line);
            while (std::getline(stream, token, ' ')) {
                const size_t pos        = token.find("=");
                const std::string key   = token.substr(0, pos);
                const std::string value = token.substr(pos + 1, std::string::npos);

                if (key == "id") {
                    charGlyph.char_id = std::stoi(value);
                    if (m_CharGlyphs.size() <= charGlyph.char_id) {
                        m_CharGlyphs.resize(charGlyph.char_id + 1);
                    }
                } else if (key == "x") {
                    charGlyph.x = std::stoi(value);
                } else if (key == "y") {
                    charGlyph.y = std::stoi(value);
                } else if (key == "width") {
                    charGlyph.width = std::stoi(value);
                } else if (key == "height") {
                    charGlyph.height = std::stoi(value);
                } else if (key == "xoffset") {
                    charGlyph.xoffset = std::stoi(value);
                } else if (key == "yoffset") {
                    charGlyph.yoffset = std::stoi(value);
                } else if (key == "xadvance") {
                    charGlyph.xadvance = std::stoi(value);
                }
            }
            if (charGlyph.yoffset + charGlyph.height > max_y_offset) {
                max_y_offset = float(charGlyph.yoffset) + float(charGlyph.height);
            }
            if (charGlyph.yoffset < min_y_offset) {
                min_y_offset = float(charGlyph.yoffset);
            }
            charGlyph.pts.emplace_back(0.0f, 0.0f, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f, 0.0f);

            float uvW1 = float(charGlyph.x) / textureWidth;
            float uvW2 = uvW1 + (float(charGlyph.width) / textureWidth);
            float uvH1 = float(charGlyph.y) / textureHeight;
            float uvH2 = uvH1 + (float(charGlyph.height) / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            m_CharGlyphs[charGlyph.char_id] = std::move(charGlyph);
        }
    }
    m_MaxHeight = max_y_offset - min_y_offset;
}

void Font::init_freetype(const std::string& filename, int height, int width) {
    uint32_t requested_char_count = 128;
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        ENGINE_PRODUCTION_LOG("ERROR::FREETYPE: Could not init FreeType Library")
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        ENGINE_PRODUCTION_LOG("ERROR::FREETYPE: Failed to load font file: " << filename)
        return;
    }
    FT_Set_Pixel_Sizes(face, (width < 0) ? 0 : width, height); //Setting the width to 0 lets the face dynamically calculate the width based on the given height.

    uint32_t max_width       = 0;
    uint32_t max_height      = 0;
    float min_y_offset       = std::numeric_limits<float>().max();
    float max_y_offset       = std::numeric_limits<float>().min();
    const auto face_height   = (face->height >> 6);

    m_CharGlyphs.resize(requested_char_count);
    for (GLubyte char_id = 0; char_id < requested_char_count; ++char_id) {
        if (FT_Load_Char(face, char_id, FT_LOAD_RENDER)) {
            ENGINE_PRODUCTION_LOG("ERROR::FREETYTPE: Failed to load Glyph: " << char_id)
            continue;
        }
        CharGlyph charGlyph = CharGlyph();
        charGlyph.char_id   = char_id;
        charGlyph.xadvance  = face->glyph->advance.x >> 6;
        charGlyph.width     = face->glyph->bitmap.width;
        charGlyph.height    = face->glyph->bitmap.rows;
        charGlyph.xoffset   = face->glyph->bitmap_left;

        charGlyph.yoffset   = -(face->glyph->bitmap_top) + ((face_height / 2) + 2);

        max_width     = glm::max(max_width, face->glyph->bitmap.width);
        max_height    = glm::max(max_height, face->glyph->bitmap.rows);

        if (charGlyph.yoffset + charGlyph.height > max_y_offset) {
            max_y_offset = float(charGlyph.yoffset) + float(charGlyph.height);
        }
        if (charGlyph.yoffset < min_y_offset) {
            min_y_offset = float(charGlyph.yoffset);
        }
        m_CharGlyphs[char_id] = std::move(charGlyph);
    }
    uint32_t final_count  = (uint32_t)glm::ceil(glm::sqrt(float(requested_char_count)));
    uint32_t final_width  = final_count * max_width;
    uint32_t final_height = final_count * max_height;

    std::vector<uint8_t> pixels(final_width * final_height * 4, 0);

    auto setPixel = [&pixels, final_width](uint32_t x, uint32_t y, const std::array<uint8_t, 4>& colors) {
        auto getIdx = [final_width](uint32_t x, uint32_t y) {
            return ((x * final_width) + y) * 4;
        };
        for (size_t i = 0; i < colors.size(); ++i) {
            pixels[getIdx(x, y) + i] = colors[i];
        }
    };

    int char_id         = 0;
    bool done           = false;
    float textureHeight = float(final_height);
    float textureWidth  = float(final_width);
    for (uint32_t i = 0; i < final_count; ++i) {
        for (uint32_t j = 0; j < final_count; ++j) {
            if (FT_Load_Char(face, char_id, FT_LOAD_RENDER)) {
                continue;
            }
            if (char_id >= m_CharGlyphs.size()) {
                done = true;
                break;
            }
            auto& charGlyph   = m_CharGlyphs[char_id];

            const auto startX = i * max_width;
            const auto startY = j * max_height;

            charGlyph.x = startX;
            charGlyph.y = startY;

            uint32_t cx               = 0;
            uint32_t glyph_x_size     = face->glyph->bitmap.width;
            uint32_t glyph_y_size     = face->glyph->bitmap.rows;
            for (uint32_t y = startY; y < startY + glyph_y_size; ++y) {
                uint32_t cy           = 0;
                uint32_t glyph_xEnd   = (startX + glyph_x_size);
                //uint32_t glyph_yEnd = (startY + glyph_y_size);
                for (uint32_t x = startX; x < glyph_xEnd; ++x) {
                    auto gray = getFromRowCol(face->glyph->bitmap.buffer, cx, cy, glyph_x_size);
                    setPixel(final_height - y - 1, x, { gray, gray, gray, gray });
                    ++cy;
                }
                ++cx;
            }
            charGlyph.pts.emplace_back(0.0f,            0.0f,             0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f,            charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f,             0.0f);

            float uvW1 =         float(charGlyph.x)      / textureWidth;
            float uvW2 = uvW1 + (float(charGlyph.width)  / textureWidth);
            float uvH1 =         float(charGlyph.y)      / textureHeight;
            float uvH2 = uvH1 + (float(charGlyph.height) / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, 1.0f - uvH2);
            charGlyph.uvs.emplace_back(uvW2, 1.0f - uvH1);
            charGlyph.uvs.emplace_back(uvW1, 1.0f - uvH1);
            charGlyph.uvs.emplace_back(uvW2, 1.0f - uvH2);

            ++char_id;
        }
        if (done) {
            break;
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    m_FontTexture = Engine::Resources::addResource<Texture>(pixels.data(), final_width, final_height, filename + "_Texture", false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);

    m_MaxHeight = max_y_offset - min_y_offset;
}
float Font::getTextHeight(std::string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    uint32_t line_count = 0;
    for (const char character : text) {
        if (character == '\n') {
            ++line_count;
        }
    }
    return (line_count == 0) ? (m_MaxHeight) : ((line_count + 1) * (m_MaxHeight + m_LineHeight));
}
float Font::getTextHeightDynamic(std::string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    float res = 0.0f;
    int min_y = std::numeric_limits<int>().max();
    int max_y = std::numeric_limits<int>().min();
    for (const char character : text) {
        if (character == '\n') {
            res += ((max_y - min_y) ) + m_LineHeight;
        } else {
            const auto& glyph = getGlyphData(character);
            min_y = std::min(min_y, glyph.yoffset);
            max_y = std::max(max_y, (glyph.yoffset) + int(glyph.height));
        }
    }
    res += ((max_y - min_y) );
    return (min_y == std::numeric_limits<int>().max() || max_y == std::numeric_limits<int>().min()) ? m_MaxHeight : res;
}
float Font::getTextWidth(std::string_view text) const {
    float row_width = 0.0f;
    float maxWidth  = 0.0f;
    for (size_t i = 0; i < text.size(); ++i) {
        const char character = text[i];
        if (character != '\0' && character != '\r') {
            if (character != '\n') {
                const CharGlyph& glyph = getGlyphData(character);
                row_width += float(glyph.xadvance);
            } else {
                //backtrack spaces
                int j = (int)i - 1;
                while (j >= 0) {
                    char character_backtrack = text[j];
                    if (character_backtrack != ' ') {
                        break;
                    }
                    const CharGlyph& glyph_space = getGlyphData(character_backtrack);
                    row_width -= (float)glyph_space.xadvance;
                    --j;
                }
                maxWidth  = std::max(maxWidth, row_width);
                row_width = 0.0f;
            }
        }
    }
    maxWidth = std::max(maxWidth, row_width);
    return maxWidth;
}
const CharGlyph& Font::getGlyphData(uint8_t character) const {
    const size_t idx = static_cast<size_t>(character);
    ASSERT(idx >= 0 && idx < m_CharGlyphs.size(), __FUNCTION__ << "(): character glyph was not found!");
    //return (idx < m_CharGlyphs.size()) ? m_CharGlyphs[idx] : m_CharGlyphs['?']; //TODO: maybe we need a check like this line?
    return m_CharGlyphs[idx];
}
