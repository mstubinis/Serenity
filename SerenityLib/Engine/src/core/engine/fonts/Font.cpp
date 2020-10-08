#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/fonts/Font.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem.hpp>

#include <ft2build.h>
#include <freetype/freetype.h> //can be replaced with #include FT_FREETYPE_H

#include <SFML/Graphics/Image.hpp>

Font* first_font = nullptr;

Font::Font(const std::string& filename, int height, int width, float line_height) 
    : Resource{ ResourceType::Font, filename }
    , m_LineHeight{ line_height }
{
    init(filename, height, width);
}
Font::Font(Font&& other) noexcept 
    : Resource(std::move(other))
    , m_FontTexture { std::exchange(other.m_FontTexture, nullptr) }
    , m_MaxHeight   { std::move(other.m_MaxHeight) }
    , m_LineHeight  { std::move(other.m_LineHeight) }
    , m_CharGlyphs  { std::move(other.m_CharGlyphs) }
{}
Font& Font::operator=(Font&& other) noexcept {
    Resource::operator=(std::move(other));
    m_FontTexture = std::exchange(other.m_FontTexture, nullptr);
    m_MaxHeight   = std::move(other.m_MaxHeight);
    m_LineHeight  = std::move(other.m_LineHeight);
    m_CharGlyphs  = std::move(other.m_CharGlyphs);
    return *this;
}
Font::~Font(){ 
}

void Font::init(const std::string& filename, int height, int width) {
    std::string extension = boost::filesystem::extension(filename);
    /*
    TODO:
        CID-keyed Type 1 fonts
        SFNT-based bitmap fonts, including color Emoji
        Type 42 fonts (limited support)
    */
    if (extension == ".fnt") { //Windows FNT fonts
        init_simple(filename, height, width);
    }else if (extension == ".ttf" || extension == ".ttc") { //TrueType fonts (TTF) and TrueType collections (TTC)
        init_freetype(filename, height, width);
    }else if (extension == ".cff") { //CFF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".pcf") { //X11 PCF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".woff") { //WOFF fonts
        init_freetype(filename, height, width);
    }else if (extension == ".otf" || extension == ".otc") { //OpenType fonts (OTF, both TrueType and CFF variants) and OpenType collections (OTC)
        init_freetype(filename, height, width);
    }else if (extension == ".pfa" || extension == ".pfb") { //Type 1 fonts (PFA and PFB)
        init_freetype(filename, height, width);
    }else if (extension == ".bdf") { //BDF fonts (including anti-aliased ones)
        init_freetype(filename, height, width);
    }else if (extension == ".pfr") { //PFR fonts
        init_freetype(filename, height, width);
    }

    if (!first_font) {
        first_font = this;
    }
}

std::vector<std::vector<std::uint8_t>> Font::generate_bitmap(const FT_GlyphSlotRec_& glyph) {
    std::vector<std::vector<std::uint8_t>> pixels;
    pixels.resize(glyph.bitmap.rows);
    for (unsigned int i = 0; i < pixels.size(); ++i) {
        pixels[i].resize(glyph.bitmap.width, 0_uc);
    }
    auto image_pixel_size = (glyph.bitmap.rows * glyph.bitmap.width) * 1; //last number = color components (so far, just grayscale)
    unsigned int row = 0;
    unsigned int col = 0;
    for (unsigned int buffer_loc = 0; buffer_loc < image_pixel_size; ++buffer_loc) {
        pixels[row][col] = glyph.bitmap.buffer[buffer_loc];
        ++col;
        if (col >= glyph.bitmap.width) {
            ++row;
            col = 0;
        }
    }
    return pixels;
}
void Font::init_simple(const std::string& filename, int height, int width) {
    std::string rawname    = filename;
    const size_t lastindex = filename.find_last_of(".");
    if (lastindex != std::string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }

    Handle handle = Engine::Resources::addResource<Texture>(rawname, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
    m_FontTexture = handle.get<Texture>();

    float min_y_offset  = std::numeric_limits<float>().max();
    float max_y_offset  = std::numeric_limits<float>().min();
    float textureHeight = (float)m_FontTexture->height();
    float textureWidth  = (float)m_FontTexture->width();

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for (std::string line; std::getline(str, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            auto charGlyph = CharGlyph();
            std::string token   = "";
            std::istringstream stream(line);
            while (std::getline(stream, token, ' ')) {
                const size_t pos        = token.find("=");
                const std::string key   = token.substr(0, pos);
                const std::string value = token.substr(pos + 1, std::string::npos);

                if (key == "id") {
                    charGlyph.char_id = std::stoi(value);
                }else if (key == "x") {
                    charGlyph.x = std::stoi(value);
                }else if (key == "y") {
                    charGlyph.y = std::stoi(value);
                }else if (key == "width") {
                    charGlyph.width = std::stoi(value);
                }else if (key == "height") {
                    charGlyph.height = std::stoi(value);
                }else if (key == "xoffset") {
                    charGlyph.xoffset = std::stoi(value);
                }else if (key == "yoffset") {
                    charGlyph.yoffset = std::stoi(value);
                }else if (key == "xadvance") {
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

            float uvW1 = (float)charGlyph.x / textureWidth;
            float uvW2 = uvW1 + ((float)charGlyph.width / textureWidth);
            float uvH1 = (float)charGlyph.y / textureHeight;
            float uvH2 = uvH1 + ((float)charGlyph.height / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            m_CharGlyphs.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(charGlyph.char_id), 
                std::forward_as_tuple(std::move(charGlyph))
            );
        }
    }
    m_MaxHeight = max_y_offset - min_y_offset;
}

void Font::init_freetype(const std::string& filename, int height, int width) {
    unsigned requested_char_count = 128;

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

    unsigned int max_width   = 0;
    unsigned int max_height  = 0;
    float min_y_offset       = std::numeric_limits<float>().max();
    float max_y_offset       = std::numeric_limits<float>().min();
    const auto face_height   = (face->height >> 6);

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
        m_CharGlyphs.emplace(char_id, std::move(charGlyph));
    }
    unsigned int final_count  = (unsigned int)glm::ceil(glm::sqrt((float)requested_char_count));
    unsigned int final_width  = final_count * max_width;
    unsigned int final_height = final_count * max_height;

    sf::Image atlas_image;
    atlas_image.create(final_width, final_height, sf::Color::Transparent);

    int char_id         = 0;
    bool done           = false;
    float textureHeight = (float)final_height;
    float textureWidth  = (float)final_width;
    for (unsigned int i = 0; i < final_count; ++i) {
        for (unsigned int j = 0; j < final_count; ++j) {
            if (FT_Load_Char(face, char_id, FT_LOAD_RENDER)) {
                continue;
            }
            if (!m_CharGlyphs.contains(char_id)) {
                done = true;
                break;
            }
            std::vector<std::vector<unsigned char>> pixels = generate_bitmap(*face->glyph);
            auto& charGlyph   = m_CharGlyphs.at(char_id);

            const auto startX = i * max_width;
            const auto startY = j * max_height;

            charGlyph.x = startX;
            charGlyph.y = startY;

            unsigned int cx               = 0;
            unsigned int glyph_y_size     = unsigned int(pixels.size());
            for (unsigned int y = startY; y < startY + glyph_y_size; ++y) {
                unsigned int cy           = 0;
                unsigned int glyph_x_size = unsigned int(pixels[cx].size());
                unsigned int glyph_xEnd   = (startX + glyph_x_size);
                unsigned int glyph_yEnd   = (startY + glyph_y_size);
                for (unsigned int x = startX; x < glyph_xEnd; ++x) {
                    auto gray = pixels[cx][cy];
                    atlas_image.setPixel(x, y, sf::Color(gray, gray, gray, gray));
                    ++cy;
                }
                ++cx;
            }
            charGlyph.pts.emplace_back(0.0f,            0.0f,             0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f,            charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f,             0.0f);

            float uvW1 =         (float)charGlyph.x      / textureWidth;
            float uvW2 = uvW1 + ((float)charGlyph.width  / textureWidth);
            float uvH1 =         (float)charGlyph.y      / textureHeight;
            float uvH2 = uvH1 + ((float)charGlyph.height / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            ++char_id;
        }
        if (done) {
            break;
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    Handle handle = Engine::Resources::addResource<Texture>(atlas_image, filename + "_Texture", false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
    m_FontTexture = handle.get<Texture>();

    m_MaxHeight = max_y_offset - min_y_offset;
}
float Font::getTextHeight(std::string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    unsigned int line_count = 0;
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
        }else {
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
            }else{
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
const CharGlyph& Font::getGlyphData(std::uint8_t character) const {
    return (m_CharGlyphs.contains(character)) ? m_CharGlyphs.at(character) : m_CharGlyphs.at('?');
}
void Font::renderText(const std::string& t, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment al, const glm::vec4& scissor){
    Engine::Renderer::renderText(t, *this, p, c, a, s, d, al, scissor);
}
void Font::renderTextStatic(const std::string& t, const glm::vec2& p, const glm::vec4& c, float a, const glm::vec2& s, float d, TextAlignment al, const glm::vec4& scissor) {
    if (first_font) {
        Engine::Renderer::renderText(t, *first_font, p, c, a, s, d, al, scissor);
    }
}
