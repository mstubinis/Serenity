#include "Font.h"
#include "Engine.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Texture.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Engine;
using namespace std;

Font::Font(string filename):EngineResource(filename){ 
    string rawname = filename;
    const size_t& lastindex = filename.find_last_of(".");
    if (lastindex != string::npos) {
        rawname = filename.substr(0, lastindex);
        rawname += ".png";
    }
    m_FontTexture = new Texture(rawname, false, ImageInternalFormat::SRGB8_ALPHA8);
    epriv::Core::m_Engine->m_ResourceManager._addTexture(m_FontTexture);

    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for (string line; getline(str, line, '\n');) {
        if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
            FontGlyph _f = FontGlyph();
            string token;
            istringstream stream(line);
            while (getline(stream, token, ' ')) {
                const size_t& pos = token.find("=");
                const string& key = token.substr(0, pos);
                const string& value = token.substr(pos + 1, string::npos);

                     if (key == "id")       _f.id = stoi(value);
                else if (key == "x")        _f.x = stoi(value);
                else if (key == "y")        _f.y = stoi(value);
                else if (key == "width")    _f.width = stoi(value);
                else if (key == "height")   _f.height = stoi(value);
                else if (key == "xoffset")  _f.xoffset = stoi(value);
                else if (key == "yoffset")  _f.yoffset = stoi(value);
                else if (key == "xadvance") _f.xadvance = stoi(value);
            }
            _f.pts.emplace_back(0.0f, 0.0f, 0.0f);
            _f.pts.emplace_back(_f.width, _f.height, 0.0f);
            _f.pts.emplace_back(0.0f, _f.height, 0.0f);
            _f.pts.emplace_back(_f.width, 0.0f, 0.0f);

            const float& textureHeight = (float)m_FontTexture->height();
            const float& textureWidth = (float)m_FontTexture->width();

            _f.uvs.emplace_back(float(_f.x / textureWidth), float(_f.y / textureHeight) + float(_f.height / textureHeight));
            _f.uvs.emplace_back(float(_f.x / textureWidth) + float(_f.width / textureWidth), float(_f.y / textureHeight));
            _f.uvs.emplace_back(float(_f.x / textureWidth), float(_f.y / textureHeight));
            _f.uvs.emplace_back(float(_f.x / textureWidth) + float(_f.width / textureWidth), float(_f.y / textureHeight) + float(_f.height / textureHeight));

            m_FontGlyphs.emplace(_f.id, std::move(_f));
        }
    }
}
Font::~Font(){ }
Texture& Font::getGlyphTexture(){ return *m_FontTexture; }
FontGlyph& Font::getGlyphData(uchar c){ return m_FontGlyphs.at(c); }
void Font::renderText(string text, glm::vec2 pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    epriv::Core::m_Engine->m_RenderManager._renderText(this,text,pos,color,scl,angle,depth);
}
