#include <core/engine/fonts/Font.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include <sstream>

using namespace Engine;
using namespace std;
#include <iostream>
Font::Font(const string& filename) : EngineResource(ResourceType::Font, filename){ 
    string rawname = filename;
    m_MaxHeight = 0.0f;
    const size_t& lastindex = filename.find_last_of(".");
    if (lastindex != string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }
    m_FontTexture = NEW Texture(rawname, false, ImageInternalFormat::SRGB8_ALPHA8);
    Handle handle = priv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

    float min_y_offset = 9999999.0f;
    float max_y_offset = 0.0f;

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for (string line; getline(str, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            FontGlyph fontGlyph = FontGlyph();
            string token;
            istringstream stream(line);
            while (getline(stream, token, ' ')) {
                const size_t& pos = token.find("=");
                const string& key = token.substr(0, pos);
                const string& value = token.substr(pos + 1, string::npos);

                if (key == "id") {
                    fontGlyph.id = stoi(value);
                }else if (key == "x") {
                    fontGlyph.x = stoi(value);
                }else if (key == "y") {
                    fontGlyph.y = stoi(value);
                }else if (key == "width") {
                    fontGlyph.width = stoi(value);
                }else if (key == "height") {
                    fontGlyph.height = stoi(value);
                }else if (key == "xoffset") {
                    fontGlyph.xoffset = stoi(value);
                }else if (key == "yoffset") {
                    fontGlyph.yoffset = stoi(value);
                }else if (key == "xadvance") {
                    fontGlyph.xadvance = stoi(value);
                }

                if (fontGlyph.yoffset + fontGlyph.height > max_y_offset) {
                    max_y_offset = static_cast<float>(fontGlyph.yoffset) + static_cast<float>(fontGlyph.height);
                }
                if (fontGlyph.yoffset < min_y_offset) {
                    min_y_offset = static_cast<float>(fontGlyph.yoffset);
                }
            }
            fontGlyph.pts.emplace_back(0.0f, 0.0f, 0.0f);
            fontGlyph.pts.emplace_back(fontGlyph.width, fontGlyph.height, 0.0f);
            fontGlyph.pts.emplace_back(0.0f, fontGlyph.height, 0.0f);
            fontGlyph.pts.emplace_back(fontGlyph.width, 0.0f, 0.0f);

            const float& textureHeight = static_cast<float>(m_FontTexture->height());
            const float& textureWidth = static_cast<float>(m_FontTexture->width());

            const float& uvW1 = static_cast<float>(fontGlyph.x) / textureWidth;
            const float& uvW2 = uvW1 + (static_cast<float>(fontGlyph.width) / textureWidth);
            const float& uvH1 = static_cast<float>(fontGlyph.y) / textureHeight;
            const float& uvH2 = uvH1 + (static_cast<float>(fontGlyph.height) / textureHeight);

            fontGlyph.uvs.emplace_back(uvW1, uvH2);
            fontGlyph.uvs.emplace_back(uvW2, uvH1);
            fontGlyph.uvs.emplace_back(uvW1, uvH1);
            fontGlyph.uvs.emplace_back(uvW2, uvH2);

            m_FontGlyphs.emplace(fontGlyph.id, std::move(fontGlyph));
        }
    }
    m_MaxHeight = max_y_offset - min_y_offset;
}
Font::~Font(){ 
}


const float Font::getTextWidth(const string& text) const {
    float row_width = 0.0f;
    float maxWidth = 0.0f;
    for (int i = 0; i < text.size(); ++i) {
        auto& character = text[i];
        if (character != '\0' && character != '\r') {
            if (character != '\n') {
                const auto& glyph = getGlyphData(character);
                row_width += (static_cast<float>(glyph.xoffset) + static_cast<float>(glyph.xadvance));
            }else{

                //backtrack spaces
                int j = i-1;
                while (j >= 0) {
                    auto& character_backtrack = text[j];
                    if (character_backtrack != ' ') {
                        break;
                    }
                    const auto& glyph_space = getGlyphData(character_backtrack);
                    row_width -= (static_cast<float>(glyph_space.xoffset) + static_cast<float>(glyph_space.xadvance));

                    --j;
                }

                if (row_width > maxWidth) {
                    maxWidth = row_width;
                }
                row_width = 0.0f;
            }
        }
    }
    if (row_width > maxWidth) {
        maxWidth = row_width;
    }
    return maxWidth;
}
const float& Font::getMaxHeight() const {
    return m_MaxHeight;
}
const float Font::getTextHeight(const string& text) const {
    if (text.empty())
        return 0.0f;
    float lineCount = 0;
    for (auto& character : text) {
        if (character == '\n') {
            ++lineCount;
        }
    }
    return (lineCount == 0) ? (m_MaxHeight) : ((lineCount+1) * m_MaxHeight);
}
const Texture& Font::getGlyphTexture() const {
    return *m_FontTexture; 
}
const FontGlyph& Font::getGlyphData(const unsigned char& character) const {
    if (!m_FontGlyphs.count(character))
        return m_FontGlyphs.at('?');
    return m_FontGlyphs.at(character);
}
void Font::renderText(const string& t, const glm::vec2& p, const glm::vec4& c, const float& a, const glm::vec2& s, const float& d, const TextAlignment::Type& al, const glm::vec4& scissor){
    Renderer::renderText(t, *this, p, c, a, s, d, al, scissor);
}
