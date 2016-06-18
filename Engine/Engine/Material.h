#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include <string>
#include <memory>

class Texture;
typedef unsigned int GLuint;
typedef char GLchar;

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
class Material final{
    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        Material(Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr);
        Material(std::string diffuse,std::string normal="",std::string glow="");
        ~Material();

        std::unordered_map<unsigned int,Texture*>& getComponents();
        Texture* getComponent(unsigned int i);
        void addComponent(unsigned int, std::string file);

        void bindTexture(unsigned int,GLuint shader,unsigned int api);

        const bool shadeless() const;
        const float glow() const;
        const float specularity() const;
        const unsigned int lightingMode() const;
        void setShadeless(bool b);
        void setGlow(float f);
        void setSpecularity(float s);
        void setLightingMode(unsigned int m);
};
#endif