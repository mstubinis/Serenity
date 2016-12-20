#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include "Engine_ResourceBasic.h"

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <boost/container/flat_set.hpp>

class Texture;
class ShaderP;
typedef unsigned int GLuint;
typedef unsigned int uint;
typedef char GLchar;

enum MATERIAL_COMPONENT_TYPE{
    MATERIAL_COMPONENT_TYPE_DIFFUSE,
    MATERIAL_COMPONENT_TYPE_NORMAL,
    MATERIAL_COMPONENT_TYPE_GLOW,
    MATERIAL_COMPONENT_TYPE_SPECULAR,
	MATERIAL_COMPONENT_TYPE_REFLECTION,
	MATERIAL_COMPONENT_TYPE_REFRACTION,
    MATERIAL_COMPONENT_TYPE_NUMBER
};
enum MATERIAL_COMPONENT_TYPE_TEXTURE_SLOTS{
    MATERIAL_COMPONENT_TEXTURE_SLOT_DIFFUSE,
    MATERIAL_COMPONENT_TEXTURE_SLOT_NORMAL,
    MATERIAL_COMPONENT_TEXTURE_SLOT_GLOW,
    MATERIAL_COMPONENT_TEXTURE_SLOT_SPECULAR,
	MATERIAL_COMPONENT_TEXTURE_SLOT_REFLECTION_CUBEMAP,
	MATERIAL_COMPONENT_TEXTURE_SLOT_REFLECTION_CUBEMAP_MAP,
	MATERIAL_COMPONENT_TEXTURE_SLOT_REFRACTION_CUBEMAP,
	MATERIAL_COMPONENT_TEXTURE_SLOT_REFRACTION_CUBEMAP_MAP,
};

enum MATERIAL_LIGHTING_MODE{
    MATERIAL_LIGHTING_MODE_BLINNPHONG,
    MATERIAL_LIGHTING_MODE_BLINN,
    MATERIAL_LIGHTING_MODE_PHONG,
    MATERIAL_LIGHTING_MODE_TANGENT,
    MATERIAL_LIGHTING_MODE_NUMBER
};

static GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MATERIAL_COMPONENT_TYPE_NUMBER] = {
    "DiffuseTexture",
    "NormalTexture",
    "GlowTexture",
	"SpecularTexture",
	"ReflectionTexture",
	"RefractionTexture"
};
static const uint MATERIAL_COUNT_LIMIT = 255;


class MaterialComponent{
	protected:
		Texture* m_Texture;
		MATERIAL_COMPONENT_TYPE m_ComponentType;
	public:
		MaterialComponent(uint type,Texture*);
		MaterialComponent(uint type,std::string& texture);
		virtual ~MaterialComponent();

		virtual void bind(GLuint shader,uint api);
		virtual void unbind(GLuint shader,uint api);

		Texture* texture() const { return m_Texture; }
		const MATERIAL_COMPONENT_TYPE type() const { return static_cast<MATERIAL_COMPONENT_TYPE>(m_ComponentType); }
};
class MaterialComponentReflection: public MaterialComponent{
	protected:
		Texture* m_Map; //the texture that maps the reflection cubemap to the object
		float m_MixFactor;
	public:
		MaterialComponentReflection(uint type,Texture* cubemap,Texture* map,float mixFactor = 0.5f);
		MaterialComponentReflection(uint type,std::string& cubemap,std::string& map,float mixFactor = 0.5f);
		~MaterialComponentReflection();

		virtual void bind(GLuint shader,uint api);
		void unbind(GLuint shader,uint api);
		void setMixFactor(float);

		const float mixFactor() const { return m_MixFactor; }
		const Texture* map() const { return m_Map; }
};
class MaterialComponentRefraction: public MaterialComponentReflection{
	private:
		float m_RefractionRatio;
	public:
		MaterialComponentRefraction(uint type,Texture* cubemap,Texture* map,float mixFactor = 0.5f,float ratio = 1.0f);
		MaterialComponentRefraction(uint type,std::string& cubemap,std::string& map,float mixFactor = 0.5f,float ratio = 1.0f);
		~MaterialComponentRefraction();

		void bind(GLuint shader,uint api);

		void setRefractionRatio(float);
		const float refractionRatio() const { return m_RefractionRatio; }
};

class Material final: public EngineResource{
	public:
		//this is very important here
		//vec4:  (  glow,  specularity, lightingMode, shadeless  )
		static std::vector<glm::vec4> m_MaterialProperities;

		static std::unordered_map<uint,std::vector<uint>> MATERIAL_TEXTURE_SLOTS_MAP;
    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        Material(std::string name, std::string diffuse,std::string normal="",std::string glow="", std::string specular="",std::string shader = "");
        Material(std::string name, Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
        ~Material();

		std::unordered_map<uint,MaterialComponent*>& getComponents();
        const MaterialComponent* getComponent(uint type) const;

		const MaterialComponentReflection* getComponentReflection() const;
		const MaterialComponentRefraction* getComponentRefraction() const;

		void bind(GLuint shader,GLuint api);

        void addComponent(uint type, Texture* texture);

		void addComponentDiffuse(Texture* texture);
		void addComponentDiffuse(std::string& textureFile);

		void addComponentNormal(Texture* texture);
		void addComponentNormal(std::string& textureFile);

		void addComponentSpecular(Texture* texture);
		void addComponentSpecular(std::string& textureFile);

		void addComponentReflection(Texture* cubeMap,Texture* map,float mixFactor = 1.0f);
		void addComponentReflection(std::string cubeMapName,std::string mapFile,float mixFactor = 1.0f);
		void addComponentReflection(std::string cubeMapTextureFiles[],std::string mapFile,float mixFactor = 1.0f);

		void addComponentRefraction(Texture* cubeMap,Texture* map,float mixFactor = 1.0f,float ratio = 1.0f);
		void addComponentRefraction(std::string cubeMapName,std::string mapFile,float mixFactor = 1.0f, float ratio = 1.0f);
		void addComponentRefraction(std::string cubeMapTextureFiles[],std::string mapFile,float mixFactor = 1.0f,float ratio = 1.0f);

        const bool shadeless() const;
        const float glow() const;
        const float specularity() const;
        const uint lightingMode() const;
		const uint id() const;
        void setShadeless(bool b);
        void setGlow(float f);
        void setSpecularity(float s);
        void setLightingMode(uint m);


		void addObject(std::string objectName);
		void removeObject(std::string objectName);
		boost::container::flat_set<std::string>& getObjects();
};
#endif