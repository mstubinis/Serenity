#include <core/engine/fonts/Font.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include <sstream>

using namespace Engine;
using namespace std;

Font* first_font = nullptr;


Font::Font(const string& filename) : EngineResource(ResourceType::Font, filename){ 
    string rawname         = filename;
    const size_t lastindex = filename.find_last_of(".");
    if (lastindex != string::npos) {
        rawname  = filename.substr(0, lastindex);
        rawname += ".png";
    }
    m_FontTexture = NEW Texture(rawname, false, ImageInternalFormat::SRGB8_ALPHA8);
    Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

    float min_y_offset = 9999999999999.0f;
    float max_y_offset = 0.0f;

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for (string line; getline(str, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            CharGlyph charGlyph = CharGlyph();
            string token;
            istringstream stream(line);
            while (getline(stream, token, ' ')) {
                const size_t pos   = token.find("=");
                const string key   = token.substr(0, pos);
                const string value = token.substr(pos + 1, string::npos);

                if (key == "id") {
                    charGlyph.id = stoi(value);
                }else if (key == "x") {
                    charGlyph.x = stoi(value);
                }else if (key == "y") {
                    charGlyph.y = stoi(value);
                }else if (key == "width") {
                    charGlyph.width = stoi(value);
                }else if (key == "height") {
                    charGlyph.height = stoi(value);
                }else if (key == "xoffset") {
                    charGlyph.xoffset = stoi(value);
                }else if (key == "yoffset") {
                    charGlyph.yoffset  = stoi(value);
                }else if (key == "xadvance") {
                    charGlyph.xadvance = stoi(value);
                }

                if (charGlyph.yoffset + charGlyph.height > max_y_offset) {
                    max_y_offset = static_cast<float>(charGlyph.yoffset) + static_cast<float>(charGlyph.height);
                }
                if (charGlyph.yoffset < min_y_offset) {
                    min_y_offset = static_cast<float>(charGlyph.yoffset);
                }
            }
            charGlyph.pts.emplace_back(0.0f, 0.0f, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(0.0f, charGlyph.height, 0.0f);
            charGlyph.pts.emplace_back(charGlyph.width, 0.0f, 0.0f);

            const float textureHeight = static_cast<float>(m_FontTexture->height());
            const float textureWidth = static_cast<float>(m_FontTexture->width());

            const float uvW1 = static_cast<float>(charGlyph.x) / textureWidth;
            const float uvW2 = uvW1 + (static_cast<float>(charGlyph.width) / textureWidth);
            const float uvH1 = static_cast<float>(charGlyph.y) / textureHeight;
            const float uvH2 = uvH1 + (static_cast<float>(charGlyph.height) / textureHeight);

            charGlyph.uvs.emplace_back(uvW1, uvH2);
            charGlyph.uvs.emplace_back(uvW2, uvH1);
            charGlyph.uvs.emplace_back(uvW1, uvH1);
            charGlyph.uvs.emplace_back(uvW2, uvH2);

            m_CharGlyphs.emplace(charGlyph.id, std::move(charGlyph));
        }
    }
    m_MaxHeight = max_y_offset - min_y_offset;
    if (!first_font) {
        first_font = this;
    }
}
Font::~Font(){ 
}

const float Font::getTextHeight(string_view text) const {
    if (text.empty()) {
        return 0.0f;
    }
    float lineCount = 0.0f;
    for (const auto& character : text) {
        if (character == '\n') {
            ++lineCount;
        }
    }
    return (lineCount == 0) ? (m_MaxHeight) : ((lineCount + 1.0f) * m_MaxHeight);
}
const float Font::getTextWidth(string_view text) const {
    float row_width = 0.0f;
    float maxWidth  = 0.0f;
    for (size_t i = 0; i < text.size(); ++i) {
        auto& character = text[i];
        if (character != '\0' && character != '\r') {
            if (character != '\n') {
                const auto& glyph = getGlyphData(character);
                row_width += (static_cast<float>(glyph.xadvance));
            }else{
                //backtrack spaces
                int j = i - 1;
                while (j >= 0) {
                    auto& character_backtrack = text[j];
                    if (character_backtrack != ' ') {
                        break;
                    }
                    const auto& glyph_space = getGlyphData(character_backtrack);
                    row_width -= (static_cast<float>(glyph_space.xadvance));
                    --j;
                }
                maxWidth = max(maxWidth, row_width);
                row_width = 0.0f;
            }
        }
    }
    maxWidth = max(maxWidth, row_width);
    return maxWidth;
}
const float Font::getMaxHeight() const {
    return m_MaxHeight;
}
const Texture& Font::getGlyphTexture() const {
    return *m_FontTexture; 
}
const CharGlyph& Font::getGlyphData(const unsigned char character) const {
    if (!m_CharGlyphs.count(character)) {
        return m_CharGlyphs.at('?');
    }
    return m_CharGlyphs.at(character);
}
void Font::renderText(const string& t, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type al, const glm::vec4& scissor){
    Renderer::renderText(t, *this, p, c, a, s, d, al, scissor);
}
void Font::renderTextStatic(const string& t, const glm::vec2& p, const glm::vec4& c, const float a, const glm::vec2& s, const float d, const TextAlignment::Type al, const glm::vec4& scissor) {
    if (first_font) {
        Renderer::renderText(t, *first_font, p, c, a, s, d, al, scissor);
    }
}
