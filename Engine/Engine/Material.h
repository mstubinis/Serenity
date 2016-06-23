#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <vector>

class Texture;
typedef unsigned int GLuint;
typedef unsigned int uint;
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

static const uint MATERIAL_COUNT_LIMIT = 255;

class Material final{
	public:
		//this is very important here
		//vec4:  (  glow,  specularity, lightingMode, shadeless    )
		static std::vector<glm::vec4> m_MaterialProperities;

    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        Material(Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr);
        Material(std::string diffuse,std::string normal="",std::string glow="");
        ~Material();

        std::unordered_map<uint,Texture*>& getComponents();
        Texture* getComponent(uint i);
        void addComponent(uint, std::string file);

        void bindTexture(uint texture,GLuint shader,uint api);

        const bool shadeless() const;
        const float glow() const;
        const float specularity() const;
        const uint lightingMode() const;
		const uint id() const;
        void setShadeless(bool b);
        void setGlow(float f);
        void setSpecularity(float s);
        void setLightingMode(uint m);
};
#endif