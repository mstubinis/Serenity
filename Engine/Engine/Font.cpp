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
        //Mesh* m_Mesh;
        Texture* m_FontTexture;
        std::unordered_map<uchar,FontGlyph*> m_FontGlyphs;

        void _init(string& _FontFilename){
            _loadTextFile(_FontFilename);


			string rawname = _FontFilename;
			size_t lastindex = _FontFilename.find_last_of("."); 
			if(lastindex != string::npos){
				rawname = _FontFilename.substr(0, lastindex); 
				rawname += ".png";
			}


            m_FontTexture = new Texture(rawname,false,ImageInternalFormat::SRGB8_ALPHA8);
            epriv::Core::m_Engine->m_ResourceManager->_addTexture(m_FontTexture);

            //Handle h = Resources::addMesh(file,1.0f,1.0f);
            //m_Mesh = Resources::getMesh(h);
        }
        void _destruct(){
            for(auto glyph:m_FontGlyphs){
                SAFE_DELETE(glyph.second);
            }
        }
        void _loadTextFile(string& filename){
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

                        if     (key == "id")       f->id = stoi(value);
                        else if(key == "x")        f->x = stoi(value);
                        else if(key == "y")        f->y = stoi(value);
                        else if(key == "width")    f->width = stoi(value);
                        else if(key == "height")   f->height = stoi(value); 
                        else if(key == "xoffset")  f->xoffset = stoi(value);
                        else if(key == "yoffset")  f->yoffset = stoi(value);
                        else if(key == "xadvance") f->xadvance = stoi(value);
                    }
                    f->m_Model = glm::mat4(1.0f);
                    f->pts.resize(4,glm::vec3(0));  f->uvs.resize(4,glm::vec2(0));

                    f->pts.at(0) = glm::vec3(0);
                    f->pts.at(1) = glm::vec3(f->width,f->height,0);
                    f->pts.at(2) = glm::vec3(0,f->height,0);
                    f->pts.at(3) = glm::vec3(f->width,0,0);

                    f->uvs.at(0) = glm::vec2(   float(f->x / 256.0f),float(f->y / 256.0f) + float(f->height / 256.0f)   );
                    f->uvs.at(1) = glm::vec2(   float(f->x / 256.0f) + float(f->width / 256.0f),float(f->y / 256.0f)   );
                    f->uvs.at(2) = glm::vec2(   float(f->x / 256.0f),float(f->y / 256.0f)   );
                    f->uvs.at(3) = glm::vec2(   float(f->x / 256.0f) + float(f->width / 256.0f),float(f->y / 256.0f) + float(f->height / 256.0f)   );

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
Texture* Font::getGlyphTexture() { return m_i->m_FontTexture; }
FontGlyph* Font::getGlyphData(uchar c){ return m_i->m_FontGlyphs.at(c); }
//Mesh* Font::getFontMesh(){ return m_i->m_Mesh; }
void Font::renderText(string text, glm::vec2& pos, glm::vec4 color,float angle, glm::vec2 scl, float depth){
    epriv::Core::m_Engine->m_RenderManager->_renderText(this,text,pos,color,scl,angle,depth);
}
