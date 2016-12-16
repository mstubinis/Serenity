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
    MATERIAL_COMPONENT_TEXTURE_SPECULAR,
	MATERIAL_COMPONENT_TEXTURE_REFLECTION,
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
    "GlowMap",
	"SpecularMap",
	"ReflectionMap"
};

static const uint MATERIAL_COUNT_LIMIT = 255;


class MaterialComponent{
	protected:
		Texture* m_Texture;
		uint m_ComponentType;
	public:
		MaterialComponent(uint type,Texture*);
		MaterialComponent(uint type,std::string texture);
		virtual ~MaterialComponent();

		virtual void bind(GLuint shader,uint api);

		const Texture* texture() const { return m_Texture; }
		const MATERIAL_COMPONENT_TYPE type() const { return static_cast<MATERIAL_COMPONENT_TYPE>(m_ComponentType); }
};
class MaterialComponentReflection: public MaterialComponent{
	private:
		Texture* m_ReflectionMap; //the texture that maps the reflection cubemap to the object
	public:
		MaterialComponentReflection(uint type,Texture* cubemap,Texture* map);
		MaterialComponentReflection(uint type,std::string cubemap,std::string map);
		~MaterialComponentReflection();

		void bind(GLuint shader,uint api);

		const Texture* reflection() const { return m_ReflectionMap; }
};

class Material final{
	public:
		//this is very important here
		//vec4:  (  glow,  specularity, lightingMode, shadeless  )
		static std::vector<glm::vec4> m_MaterialProperities;

    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        Material(Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr);
        Material(std::string diffuse,std::string normal="",std::string glow="", std::string specular="");
        ~Material();

		std::unordered_map<uint,MaterialComponent*>& getComponents();
        MaterialComponent* getComponent(uint type);
        void addComponent(uint type, Texture* texture);

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