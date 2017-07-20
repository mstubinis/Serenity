#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#define MATERIAL_COUNT_LIMIT 255.0

#include "BindableResource.h"
#include <glm/glm.hpp>
#include <unordered_map>

class MeshInstance;
class Texture;
class ShaderP;
class Mesh;
typedef unsigned int GLuint;
typedef unsigned int uint;
typedef char GLchar;

class MaterialComponentType{
    public: enum Type{
        DIFFUSE,
        NORMAL,
        GLOW,
        SPECULAR,
        REFLECTION,
        REFRACTION,

        AO,
        METALNESS,
        ROUGHNESS,

        NUMBER
    };
};
class MaterialComponentTextureSlot{
    public: enum Slot{
        DIFFUSE,
        NORMAL,
        GLOW,
        SPECULAR,

        AO,
        METALNESS,
        ROUGHNESS,

        REFLECTION_CUBEMAP,
        REFLECTION_CUBEMAP_MAP,

        REFRACTION_CUBEMAP,
        REFRACTION_CUBEMAP_MAP,
    };
};

static GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MaterialComponentType::Type::NUMBER] = {
    "DiffuseTexture",
    "NormalTexture",
    "GlowTexture",
    "SpecularTexture",
    "AOTexture",
    "MetalnessTexture",
    "RoughnessTexture",
    "ReflectionTexture",
    "RefractionTexture"
};

class MaterialComponent{
    protected:
        Texture* m_Texture;
        MaterialComponentType::Type m_ComponentType;
    public:
        MaterialComponent(uint type,Texture*);
        MaterialComponent(uint type,std::string& texture);
        virtual ~MaterialComponent();

        virtual void bind();
        virtual void unbind();

        Texture* texture() const { return m_Texture; }
        const MaterialComponentType::Type type() const { return static_cast<MaterialComponentType::Type>(m_ComponentType); }
};
class MaterialComponentAO: public MaterialComponent{
    protected:
        float m_AOBaseValue;
    public:
        MaterialComponentAO(Texture* texture,float aoBaseValue);
        MaterialComponentAO(std::string& texture,float aoBaseValue);
        ~MaterialComponentAO();

        virtual void bind();
        void unbind();
        void setAOBaseValue(float);

        const float aoBaseValue() const { return m_AOBaseValue; }
};
class MaterialComponentMetalness: public MaterialComponent{
    protected:
        float m_MetalnessBaseValue;
    public:
        MaterialComponentMetalness(Texture* texture,float metalnessBaseValue);
        MaterialComponentMetalness(std::string& texture,float metalnessBaseValue);
        ~MaterialComponentMetalness();

        virtual void bind();
        void unbind();
        void setMetalnessBaseValue(float);

        const float metalnessBaseValue() const { return m_MetalnessBaseValue; }
};
class MaterialComponentRoughness: public MaterialComponent{
    protected:
        float m_RoughnessBaseValue;
    public:
        MaterialComponentRoughness(Texture* texture,float roughnessBaseValue);
        MaterialComponentRoughness(std::string& texture,float roughnessBaseValue);
        ~MaterialComponentRoughness();

        virtual void bind();
        void unbind();
        void setRoughnessBaseValue(float);

        const float roughnessBaseValue() const { return m_RoughnessBaseValue; }
};
class MaterialComponentReflection: public MaterialComponent{
    protected:
        Texture* m_Map; //the texture that maps the reflection cubemap to the object
        float m_MixFactor;
    public:
        MaterialComponentReflection(uint type,Texture* cubemap,Texture* map,float mixFactor);
        MaterialComponentReflection(uint type,std::string& cubemap,std::string& map,float mixFactor);
        ~MaterialComponentReflection();

        virtual void bind();
        void unbind();
        void setMixFactor(float);

        const float mixFactor() const { return m_MixFactor; }
        const Texture* map() const { return m_Map; }
};
class MaterialComponentRefraction: public MaterialComponentReflection{
    private:
        float m_RefractionRatio;
    public:
        MaterialComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float ratio);
        MaterialComponentRefraction(std::string& cubemap,std::string& map,float mixFactor,float ratio);
        ~MaterialComponentRefraction();

        void bind();

        void setRefractionRatio(float);
        const float refractionRatio() const { return m_RefractionRatio; }
};

class MaterialMeshEntry{
    private:
        Mesh* m_Mesh;
        std::unordered_map<std::string,std::vector<MeshInstance*>> m_MeshInstances;
        public:
        MaterialMeshEntry(Mesh*);
        ~MaterialMeshEntry();

        Mesh* mesh(){ return m_Mesh; }
        std::unordered_map<std::string,std::vector<MeshInstance*>>& meshInstances(){ return m_MeshInstances; }

        void addMeshInstance(std::string& objectName,MeshInstance*);
        void removeMeshInstance(std::string& objectName,MeshInstance*);
};

class Material final: public BindableResource{
    public: enum LightingMode{
        BLINNPHONG,
        PHONG,
        GXX,
        COOKTORRANCE,
        GAUSSIAN,
        BECKMANN,
        PBR,
        NUMBER
    };


    public:
        static std::vector<glm::vec4> m_MaterialProperities;

        static std::unordered_map<uint,std::vector<uint>> MATERIAL_TEXTURE_SLOTS_MAP;
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        Material(std::string name, std::string diffuse,std::string normal="",std::string glow="", std::string specular="",std::string shader = "");
        Material(std::string name, Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
        ~Material();

        const std::unordered_map<uint,MaterialComponent*>& getComponents() const;
        const MaterialComponent* getComponent(uint type) const;

        const MaterialComponentReflection* getComponentReflection() const;
        const MaterialComponentRefraction* getComponentRefraction() const;

        void addComponent(uint type, Texture* texture);

        void addComponentDiffuse(Texture* texture);
        void addComponentDiffuse(std::string& textureFile);

        void addComponentNormal(Texture* texture);
        void addComponentNormal(std::string& textureFile);

        void addComponentGlow(Texture* texture);
        void addComponentGlow(std::string& textureFile);

        void addComponentSpecular(Texture* texture);
        void addComponentSpecular(std::string& textureFile);

        void addComponentAO(Texture* texture,float baseValue = 1.0f);
        void addComponentAO(std::string& textureFile,float baseValue = 1.0f);

        void addComponentMetalness(Texture* texture,float baseValue = 1.0f);
        void addComponentMetalness(std::string& textureFile,float baseValue = 1.0f);

        void addComponentRoughness(Texture* texture,float baseValue = 1.0f);
        void addComponentRoughness(std::string& textureFile,float baseValue = 1.0f);

        void addComponentReflection(Texture* cubeMap,Texture* map,float mixFactor = 1.0f);
        void addComponentReflection(std::string cubeMapName,std::string mapFile,float mixFactor = 1.0f);
        void addComponentReflection(std::string cubeMapTextureFiles[],std::string mapFile,float mixFactor = 1.0f);

        void addComponentRefraction(Texture* cubeMap,Texture* map,float mixFactor = 1.0f,float ratio = 1.0f);
        void addComponentRefraction(std::string cubeMapName,std::string mapFile,float mixFactor = 1.0f, float ratio = 1.0f);
        void addComponentRefraction(std::string cubeMapTextureFiles[],std::string mapFile,float mixFactor = 1.0f,float ratio = 1.0f);

        const bool shadeless() const;
        const float glow() const;
		const float frensel() const;
        const float specularity() const;
        const uint lightingMode() const;
        const uint id() const;
		void setFrensel(float f);
        void setShadeless(bool b);
        void setGlow(float f);
        void setSpecularity(float s);
        void setLightingMode(uint m);

        void bind();
        void unbind();

        void load();
        void unload();

        void addMesh(std::string meshName);
        void removeMesh(std::string meshName);
        std::vector<MaterialMeshEntry*>& getMeshes();
};
#endif
