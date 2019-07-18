#include <core/engine/fonts/Font.h>
#include <core/engine/Engine.h>
#include <core/engine/textures/Texture.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Engine;
using namespace std;

Font::Font(const string& filename):EngineResource(filename){ 
    string rawname = filename;
    const size_t& lastindex = filename.find_last_of(".");
    if (lastindex != string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }
    m_FontTexture = new Texture(rawname, false, ImageInternalFormat::SRGB8_ALPHA8);
    Handle handle = epriv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

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

                     if (key == "id")       fontGlyph.id = stoi(value);
                else if (key == "x")        fontGlyph.x = stoi(value);
                else if (key == "y")        fontGlyph.y = stoi(value);
                else if (key == "width")    fontGlyph.width = stoi(value);
                else if (key == "height")   fontGlyph.height = stoi(value);
                else if (key == "xoffset")  fontGlyph.xoffset = stoi(value);
                else if (key == "yoffset")  fontGlyph.yoffset = stoi(value);
                else if (key == "xadvance") fontGlyph.xadvance = stoi(value);
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
}
Font::~Font(){ 
}


const float Font::getTextWidth(const string& text) const {
    float ret = 0;
    float maxWidth = 0;
    for (auto& character : text) {
        if (character != '\0') {
            if (character != '\n') {
                const auto& glyph = getGlyphData(character);
                ret += glyph.xadvance;
            }else{
                if (ret > maxWidth) {
                    maxWidth = ret;
                    ret = 0;
                }
            }
        }
    }
    if (ret > maxWidth) {
        maxWidth = ret;
    }
    return maxWidth;
}
const float Font::getTextHeight(const string& text) const {
    float lineCount = 1;
    for (auto& character : text) {
        if (character == '\n') {
            ++lineCount;
        }
    }
    auto chosenChar = getGlyphData('X').height + 2;
    return lineCount * chosenChar;
}
const Texture& Font::getGlyphTexture() const {
    return *m_FontTexture; 
}
const FontGlyph& Font::getGlyphData(const uchar& character) const {
    if (!m_FontGlyphs.count(character))
        return m_FontGlyphs.at('?');
    return m_FontGlyphs.at(character);
}
void Font::renderText(const string& t, const glm::vec2& p, const glm::vec4& c, const float& a, const glm::vec2& s, const float& d, const TextAlignment::Type& al){
    Renderer::renderText(t, *this, p, c, a, s, d, al);
}
