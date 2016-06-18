#include <unordered_map>
#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"

using namespace Engine;


class Material::impl final{
    public:
        std::unordered_map<unsigned int,Texture*> m_Components;
        unsigned int m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_Specularity;

        void _init(Texture* diffuse,Texture* normal,Texture* glow){
            for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++)
                m_Components[i] = nullptr;
            m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = diffuse;
            m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = normal;
            m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = glow;

            m_Shadeless = false;
            m_BaseGlow = 0.0f;
            m_Specularity = 1.0f;
            m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
        }
        void _init(std::string& diffuse, std::string& normal, std::string& glow){
            Texture* diffuseT = Resources::getTexture(diffuse); 
            Texture* normalT = Resources::getTexture(normal); 
            Texture* glowT = Resources::getTexture(glow);
            if(diffuseT == nullptr && diffuse != "") diffuseT = new Texture(diffuse);
            if(normalT == nullptr && normal != "")  normalT = new Texture(normal);
            if(glowT == nullptr && glow != "")    glowT = new Texture(glow);
            _init(diffuseT,normalT,glowT);
        }
        void _destruct(){
        }
        void _addComponent(unsigned int type, std::string& file){
            if(m_Components[type] != nullptr)
                return;
            m_Components[type] = new Texture(file);
        }
        void _bindTexture(unsigned int c,GLuint shader,unsigned int api){
            if(api == ENGINE_RENDERING_API_OPENGL){
                std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[c];
                if(m_Components[c] == nullptr || m_Components[c]->address() == 0){//Texture / Material type not present; disable this material
                    glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
                    return;
                }
                glActiveTexture(GL_TEXTURE0 + c);
                glBindTexture(m_Components[c]->type(), m_Components[c]->address());
                glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), c);
                glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
            }
            else if(api == ENGINE_RENDERING_API_DIRECTX){
            }
        }
        void setShadeless(bool b){ m_Shadeless = b; }
        void setBaseGlow(float f){ m_BaseGlow = f; }
        void setSpecularity(float s){ m_Specularity = s; }
        void setLightingMode(unsigned int m){ m_LightingMode = m; }
};

Material::Material(Texture* diffuse,Texture* normal,Texture* glow):m_i(new impl()){
    m_i->_init(diffuse,normal,glow);
}
Material::Material(std::string diffuse, std::string normal, std::string glow):m_i(new impl()){
    m_i->_init(diffuse,normal,glow);
}
Material::~Material(){
    m_i->_destruct();
}
void Material::addComponent(unsigned int type, std::string file){
    m_i->_addComponent(type,file);
}
void Material::bindTexture(unsigned int c,GLuint shader,unsigned int api){
    m_i->_bindTexture(c,shader,api);
}
std::unordered_map<unsigned int,Texture*>& Material::getComponents(){ 
    return m_i->m_Components;
}
Texture* Material::getComponent(unsigned int i){ 
    return m_i->m_Components[i];
}
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::specularity() const { return m_i->m_Specularity; }
const unsigned int Material::lightingMode() const { return m_i->m_LightingMode; }
void Material::setShadeless(bool b){ m_i->m_Shadeless = b; }
void Material::setGlow(float f){ m_i->m_BaseGlow = f; }
void Material::setSpecularity(float s){ m_i->m_Specularity = s; }
void Material::setLightingMode(unsigned int m){ m_i->m_LightingMode = m; }