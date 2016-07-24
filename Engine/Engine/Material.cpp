#include <unordered_map>
#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"

using namespace Engine;

std::vector<glm::vec4> Material::m_MaterialProperities;

class Material::impl final{
    public:
        std::unordered_map<uint,Texture*> m_Components;
        uint m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_SpecularityPower;
		uint m_ID;
        void _init(Texture* diffuse,Texture* normal,Texture* glow,Texture* specular){
            for(uint i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++)
                m_Components[i] = nullptr;
            m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = diffuse;
            m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = normal;
            m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = glow;
			m_Components[MATERIAL_COMPONENT_TEXTURE_SPECULAR] = specular;

            m_Shadeless = false;
            m_BaseGlow = 0.0f;
            m_SpecularityPower = 50.0f;
            m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
			_addToMaterialPool();
        }
        void _init(std::string& diffuse, std::string& normal, std::string& glow, std::string& specular){
            Texture* diffuseT = Resources::getTexture(diffuse); 
            Texture* normalT = Resources::getTexture(normal); 
            Texture* glowT = Resources::getTexture(glow);
			Texture* specularT = Resources::getTexture(specular);
            if(diffuseT == nullptr && diffuse != "") diffuseT = new Texture(diffuse);
            if(normalT == nullptr && normal != "")  normalT = new Texture(normal);
            if(glowT == nullptr && glow != "")    glowT = new Texture(glow);
			if(specularT == nullptr && specular != "")    specularT = new Texture(specular);
            _init(diffuseT,normalT,glowT,specularT);
        }
		void _addToMaterialPool(){
			this->m_ID = Material::m_MaterialProperities.size();
			Material::m_MaterialProperities.push_back(glm::vec4(m_BaseGlow,m_SpecularityPower,m_LightingMode,m_Shadeless));
		}
		void _updateGlobalMaterialPool(){
			glm::vec4& ref = Material::m_MaterialProperities.at(m_ID);
			ref.r = m_BaseGlow;
			ref.g = m_SpecularityPower;
			ref.b = float(m_LightingMode);
			ref.a = m_Shadeless;
		}
        void _destruct(){
        }
        void _addComponent(uint type, std::string& file){
            if(m_Components[type] != nullptr)
                return;
            m_Components[type] = new Texture(file);
        }
        void _bindTexture(uint c,GLuint shader,uint api){
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
        void _setShadeless(bool& b){ m_Shadeless = b; _updateGlobalMaterialPool(); }
        void _setBaseGlow(float& f){ m_BaseGlow = f; _updateGlobalMaterialPool(); }
        void _setSpecularity(float& s){ m_SpecularityPower = s; _updateGlobalMaterialPool(); }
        void _setLightingMode(uint& m){ m_LightingMode = m; _updateGlobalMaterialPool(); }
};

Material::Material(Texture* diffuse,Texture* normal,Texture* glow,Texture* specular):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
}
Material::Material(std::string diffuse, std::string normal, std::string glow,std::string specular):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
}
Material::~Material(){
    m_i->_destruct();
}
void Material::addComponent(uint type, std::string file){
    m_i->_addComponent(type,file);
}
void Material::bindTexture(uint c,GLuint shader,unsigned int api){
    m_i->_bindTexture(c,shader,api);
}
std::unordered_map<uint,Texture*>& Material::getComponents(){ 
    return m_i->m_Components;
}
Texture* Material::getComponent(uint index){ 
    return m_i->m_Components[index];
}
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::specularity() const { return m_i->m_SpecularityPower; }
const uint Material::lightingMode() const { return m_i->m_LightingMode; }
const uint Material::id() const { return m_i->m_ID; }

void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setSpecularity(float s){ m_i->_setSpecularity(s); }
void Material::setLightingMode(uint m){ m_i->_setLightingMode(m); }