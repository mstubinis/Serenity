#include "Font.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"
#include <sstream>
#include <GLM/gtc/type_ptr.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

using namespace Engine;
using namespace std;

FontData::FontData(string& filename){
    _loadTextFile(filename);
    string texture_filename = filename.substr(0,filename.size()-4);
    texture_filename += ".png";
    m_FontTexture = new Texture(texture_filename);
}
FontData::~FontData(){
    for(auto glyph:m_FontGlyphs){
        SAFE_DELETE(glyph.second);
    }
}
FontGlyph* FontData::getGlyphData(unsigned char c){ return m_FontGlyphs[c]; }
void FontData::_loadTextFile(string& filename){
    unordered_map<unsigned char,FontGlyph*> _Font_Chars;
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for(string line; getline(str, line, '\n');){
        if(line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' '){
            FontGlyph* font = new FontGlyph();
            string token;
            istringstream stream(line);
            while(getline(stream, token, ' ')) {
                size_t pos = token.find("=");

                string key = token.substr(0, pos);
                string value = token.substr(pos + 1, string::npos);

                if(key == "id")            font->id = stoi(value);
                else if(key == "x")        font->x = stoi(value);
                else if(key == "y")        font->y = stoi(value);
                else if(key == "width")    font->width = stoi(value);
                else if(key == "height")   font->height = stoi(value); 
                else if(key == "xoffset")  font->xoffset = stoi(value);
                else if(key == "yoffset")  font->yoffset = stoi(value);
                else if(key == "xadvance") font->xadvance = stoi(value);
            }
            font->m_Model = glm::mat4(1);
            string name = filename.substr(0,filename.size()-4);
            Resources::addMesh(name + "_" + boost::lexical_cast<std::string>(font->id),
                static_cast<float>(font->x),
                static_cast<float>(font->y),
                static_cast<float>(font->width),
                static_cast<float>(font->height));
            font->char_mesh = Resources::getMesh(name + "_" + boost::lexical_cast<string>(font->id));
            _Font_Chars[font->id] = font;
        }
    }
    m_FontGlyphs = _Font_Chars;
}

Font::Font(string filename):EngineResource(filename){
    m_FontData = new FontData(filename);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Fonts,name(),boost::shared_ptr<Font>(this));
}
Font::~Font(){
    SAFE_DELETE(m_FontData);
}
void Font::renderText(string& text, glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    Engine::Renderer::Detail::RenderManagement::getFontRenderQueue().push_back(FontRenderInfo(name(),text,pos,color,scl,angle,depth));
}
