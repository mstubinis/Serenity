#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include "BindableResource.h"
#include <unordered_map>

#include <glm/vec4.hpp>

class MeshInstance;
class Texture;
class ShaderP;
class Mesh;
typedef unsigned int GLuint;
typedef unsigned int uint;
typedef char GLchar;

class MaterialComponentType{
    public: enum Type{
        Diffuse,
        Normal,
        Glow,
        Specular,
        Reflection,
        Refraction,

        AO,
        Metalness,
        Smoothness,

        Number
    };
};
class MaterialComponentTextureSlot{
    public: enum Slot{
        Diffuse,
        Normal,
        Glow,
        Specular,

        AO,
        Metalness,
        Smoothness,

        Reflection_CUBEMAP,
        Reflection_CUBEMAP_MAP,

        Refraction_CUBEMAP,
        Refraction_CUBEMAP_MAP,
    };
};
class FrenselColor{
    public: enum Color{
        Water,
        Plastic_Or_Glass_Low,
        Plastic_High,
        Glass_Or_Ruby_High,
        Diamond,
        Iron,
        Copper,
        Gold,
        Aluminium,
        Silver,
        Black_Leather,
        Yellow_Paint_MERL,
        Chromium,
        Red_Plastic_MERL,
        Blue_Rubber_MERL,
        Zinc,
        Car_Paint_Orange
    };
};



class DiffuseModel{
    public: enum Model{
        Lambert,
        Oren_Nayar,
        Ashikhmin_Shirley,
        Minnaert
    };
};
class SpecularModel{
    public: enum Model{
        Blinn_Phong,
        Phong,
        GXX,
        Cook_Torrance,
        Guassian,
        Beckmann,
        Ashikhmin_Shirley
    };
};


static GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MaterialComponentType::Type::Number] = {
    "DiffuseTexture",
    "NormalTexture",
    "GlowTexture",
    "SpecularTexture",
    "AOTexture",
    "MetalnessTexture",
    "SmoothnessTexture",
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
        float m_RefractionIndex;
    public:
        MaterialComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float ratio);
        MaterialComponentRefraction(std::string& cubemap,std::string& map,float mixFactor,float ratio);
        ~MaterialComponentRefraction();

        void bind();

        void setRefractionIndex(float);
        const float refractionIndex() const { return m_RefractionIndex; }
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

        void addMeshInstance(const std::string objectName,MeshInstance*);
        void removeMeshInstance(const std::string objectName,MeshInstance*);
};

class Material final: public BindableResource{
    public: 
		static void setAllDiffuseModels(DiffuseModel::Model);
		static void setAllSpecularModels(SpecularModel::Model);
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
        void addComponentDiffuse(std::string textureFile);

        void addComponentNormal(Texture* texture);
        void addComponentNormal(std::string textureFile);

        void addComponentGlow(Texture* texture);
        void addComponentGlow(std::string textureFile);

        void addComponentSpecular(Texture* texture);
        void addComponentSpecular(std::string textureFile);

        void addComponentAO(Texture* texture,float baseValue = 1.0f);
        void addComponentAO(std::string textureFile,float baseValue = 1.0f);

        void addComponentMetalness(Texture* texture,float baseValue = 1.0f);
        void addComponentMetalness(std::string textureFile,float baseValue = 1.0f);

        void addComponentSmoothness(Texture* texture,float baseValue = 1.0f);
        void addComponentSmoothness(std::string textureFile,float baseValue = 1.0f);

        void addComponentReflection(Texture* cubeMap,Texture* map,float mixFactor = 1.0f);
        void addComponentReflection(std::string cubeMapName,std::string mapFile,float mixFactor = 1.0f);
        void addComponentReflection(std::string cubeMapTextureFiles[],std::string mapFile,float mixFactor = 1.0f);

        void addComponentRefraction(Texture* cubeMap,Texture* map,float refractiveIndex = 1.0f,float mixFactor = 1.0f);
        void addComponentRefraction(std::string cubeMapName,std::string mapFile,float refractiveIndex = 1.0f, float mixFactor = 1.0f);
        void addComponentRefraction(std::string cubeMapTextureFiles[],std::string mapFile,float refractiveIndex = 1.0f,float mixFactor = 1.0f);

        const uint id() const;
    
        const glm::vec3& frensel() const;
        const bool shadeless() const;
        const float glow() const;
        const float smoothness() const;
        const float metalness() const;
        const float ao() const;
        
        void setFrensel(glm::vec3);
		void setFrensel(FrenselColor::Color);
        void setShadeless(bool);
        void setGlow(float);
        void setSmoothness(float);
        void setAO(float);
        void setMetalness(float);
    
        const uint specularModel() const;
        void setSpecularModel(SpecularModel::Model m);
        const uint diffuseModel() const;    
        void setDiffuseModel(DiffuseModel::Model m);

        void bind();
        void unbind();

        void load();
        void unload();

        void addMeshEntry(std::string meshName);
        void removeMeshEntry(std::string meshName);
        std::vector<MaterialMeshEntry*>& getMeshEntries();
};
#endif
