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
    string file = filename.substr(0,filename.size()-4);
    file += ".png";
    m_FontTexture = new Texture(file,"",GL_TEXTURE_2D,false,ImageInternalFormat::SRGB8_ALPHA8);
}
FontData::~FontData(){
    for(auto glyph:m_FontGlyphs){
        SAFE_DELETE(glyph.second);
    }
}
FontGlyph* FontData::getGlyphData(uchar c){ return m_FontGlyphs[c]; }
void FontData::_loadTextFile(string& filename){
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    for(string line; getline(str, line, '\n');){
        if(line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' '){
            FontGlyph* f = new FontGlyph();
            string token;
            istringstream stream(line);
            while(getline(stream, token, ' ')) {
                size_t pos = token.find("=");

                string key = token.substr(0, pos);
                string value = token.substr(pos + 1, string::npos);

                if(key == "id")            f->id = stoi(value);
                else if(key == "x")        f->x = stoi(value);
                else if(key == "y")        f->y = stoi(value);
                else if(key == "width")    f->width = stoi(value);
                else if(key == "height")   f->height = stoi(value); 
                else if(key == "xoffset")  f->xoffset = stoi(value);
                else if(key == "yoffset")  f->yoffset = stoi(value);
                else if(key == "xadvance") f->xadvance = stoi(value);
            }
			f->m_Model = Renderer::Detail::RenderManagement::m_IdentityMat4;
            string name = filename.substr(0,filename.size()-4);
            Resources::addMesh(name+"_"+to_string(f->id),float(f->x),float(f->y),float(f->width),float(f->height));
            f->char_mesh = Resources::getMesh(name+"_"+to_string(f->id));
            m_FontGlyphs.emplace(f->id,f);
        }
    }
}

Font::Font(string filename):EngineResource(filename){
    m_FontData = new FontData(filename);
    Resources::Detail::ResourceManagement::_addToContainer(Resources::Detail::ResourceManagement::m_Fonts,name(),boost::shared_ptr<Font>(this));
}
Font::~Font(){
    SAFE_DELETE(m_FontData);
}
void Font::renderText(string text, glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    Engine::Renderer::Detail::RenderManagement::getFontRenderQueue().push_back(FontRenderInfo(name(),text,pos,color,scl,angle,depth));
}
