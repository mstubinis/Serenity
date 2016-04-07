#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>

enum MATERIAL_COMPONENT_TYPE{
    MATERIAL_COMPONENT_TEXTURE_DIFFUSE,
    MATERIAL_COMPONENT_TEXTURE_NORMAL,
    MATERIAL_COMPONENT_TEXTURE_GLOW,
    MATERIAL_COMPONENT_TYPE_NUMBER
};
enum MATERIAL_LIGHTING_MODE{
    MATERIAL_LIGHTING_MODE_BLINNPHONG,
    MATERIAL_LIGHTING_MODE_BLINN,
    MATERIAL_LIGHTING_MODE_PHONG,
    MATERIAL_LIGHTING_MODE_TANGENT,
    MATERIAL_LIGHTING_MODE_NUMBER
};

static GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MATERIAL_COMPONENT_TYPE_NUMBER] = {
    "DiffuseMap",
    "NormalMap",
    "GlowMap"
};
class Texture;
class Material final{
    private:
        void _init(Texture*,Texture*,Texture*);
        std::unordered_map<unsigned int,Texture*> m_Components;
        unsigned int m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_Specularity;
    public:
        Material(Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr);
        Material(std::string diffuse,std::string normal="",std::string glow="");
        ~Material();

        std::unordered_map<unsigned int,Texture*>& getComponents(){ return m_Components; }
        Texture* getComponent(unsigned int i){ return m_Components[i]; }
        void addComponent(unsigned int, std::string file);

        void bindTexture(unsigned int,GLuint shader,unsigned int api);

        const bool getShadeless() const { return m_Shadeless; }
        const float getBaseGlow() const { return m_BaseGlow; }
        const float getSpecularity() const { return m_Specularity; }
        const unsigned int getLightingMode() const { return m_LightingMode; }
        void setShadeless(bool b){ m_Shadeless = b; }
        void setBaseGlow(float f){ m_BaseGlow = f; }
        void setSpecularity(float s){ m_Specularity = s; }
        void setLightingMode(unsigned int m){ m_LightingMode = m; }
};
#endif