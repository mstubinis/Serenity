#include "Engine.h"
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

class Font::impl final{
    public:
        Texture* m_FontTexture;
        unordered_map<uchar,FontGlyph*> m_FontGlyphs;

        void _init(const string& _FontFilename){
            _loadTextFile(_FontFilename);

            string rawname = _FontFilename;
            const size_t& lastindex = _FontFilename.find_last_of("."); 
            if(lastindex != string::npos){
                rawname = _FontFilename.substr(0, lastindex); 
                rawname += ".png";
            }
            m_FontTexture = new Texture(rawname,false,ImageInternalFormat::SRGB8_ALPHA8);
            epriv::Core::m_Engine->m_ResourceManager->_addTexture(m_FontTexture);
        }
        void _destruct(){
            for(auto glyph:m_FontGlyphs){
                SAFE_DELETE(glyph.second);
            }
        }
        void _loadTextFile(const string& filename){
            boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
            for(string line; getline(str, line, '\n');){
                if(line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' '){
                    FontGlyph* f = new FontGlyph();
					FontGlyph& _f = *f;
                    string token;
                    istringstream stream(line);
                    while(getline(stream, token, ' ')) {
                        const size_t& pos = token.find("=");
                        const string& key = token.substr(0, pos);
                        const string& value = token.substr(pos + 1, string::npos);

                        if     (key == "id")       _f.id = stoi(value);
                        else if(key == "x")        _f.x = stoi(value);
                        else if(key == "y")        _f.y = stoi(value);
                        else if(key == "width")    _f.width = stoi(value);
                        else if(key == "height")   _f.height = stoi(value); 
                        else if(key == "xoffset")  _f.xoffset = stoi(value);
                        else if(key == "yoffset")  _f.yoffset = stoi(value);
                        else if(key == "xadvance") _f.xadvance = stoi(value);
                    }
					_f.pts.emplace_back(0.0f,0.0f,0.0f);
					_f.pts.emplace_back(f->width,f->height,0.0f);
					_f.pts.emplace_back(0.0f,f->height,0.0f);
					_f.pts.emplace_back(f->width,0.0f,0.0f);

					_f.uvs.emplace_back(float(f->x / 256.0f),float(f->y / 256.0f) + float(f->height / 256.0f));
					_f.uvs.emplace_back(float(f->x / 256.0f) + float(f->width / 256.0f),float(f->y / 256.0f));
					_f.uvs.emplace_back(float(f->x / 256.0f),float(f->y / 256.0f));
					_f.uvs.emplace_back(float(f->x / 256.0f) + float(f->width / 256.0f),float(f->y / 256.0f) + float(f->height / 256.0f));

                    m_FontGlyphs.emplace(f->id,f);
                }
            }
        }
};
Font::Font(string filename):EngineResource(filename),m_i(new impl){
    m_i->_init(filename);
}
Font::~Font(){
    m_i->_destruct();
}
Texture* Font::getGlyphTexture(){ return m_i->m_FontTexture; }
FontGlyph* Font::getGlyphData(uchar c){ return m_i->m_FontGlyphs.at(c); }
void Font::renderText(string text, glm::vec2 pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    epriv::Core::m_Engine->m_RenderManager->_renderText(this,text,pos,color,scl,angle,depth);
}
