#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include "BindableResource.h"
#include <unordered_map>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <glm/vec4.hpp>

class MeshInstance;
class Texture;
class ShaderP;
class Mesh;
typedef unsigned int uint;

class MaterialComponentType{public: enum Type{
    Diffuse,
    Normal,
    Glow,
    Specular,
    AO,
    Metalness,
    Smoothness,  
    Reflection,
    Refraction,
	ParallaxOcclusion,

    Number,
};};
class MaterialComponentTextureSlot{public: enum Slot{
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
	Heightmap,
};};
class MaterialPhysics{public: enum Physics{
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
    Car_Paint_Orange,
    Skin,
    Quartz,
    Crystal,
    Alcohol,
    Milk,
    Glass,
    Titanium,
    Platinum,
    Nickel,
};};
class DiffuseModel{public: enum Model{
    Lambert,
    Oren_Nayar,
    Ashikhmin_Shirley,
    Minnaert,
};};
class SpecularModel{public: enum Model{
    Blinn_Phong,
    Phong,
    GXX,
    Cook_Torrance,
    Guassian,
    Beckmann,
    Ashikhmin_Shirley,
};};
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
class MaterialComponentParallaxOcclusion: public MaterialComponent{
    protected:
        float m_HeightScale;
    public:
        MaterialComponentParallaxOcclusion(Texture* map,float heightScale);
        MaterialComponentParallaxOcclusion(std::string& map,float heightScale);
        ~MaterialComponentParallaxOcclusion();

        void bind();
        void unbind();
        void setHeightScale(float);

        const float heightScale() const { return m_HeightScale; }
};
class MaterialMeshEntry{
    private:
		class impl; std::unique_ptr<impl> m_i;
    public:
        MaterialMeshEntry(Mesh*);
        ~MaterialMeshEntry();

        Mesh* mesh();
        std::unordered_map<std::string,std::vector<MeshInstance*>>& meshInstances();

        void addMeshInstance(const std::string& objectName,MeshInstance*);
        void removeMeshInstance(const std::string& objectName,MeshInstance*);
};

class Material final: public BindableResource{
    public: 
        static void setAllDiffuseModels(DiffuseModel::Model);
        static void setAllSpecularModels(SpecularModel::Model);
        static std::vector<glm::vec4> m_MaterialProperities;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Material(std::string name, std::string diffuse,std::string normal="",std::string glow="", std::string specular="",std::string shader = "");
        Material(std::string name, Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr,ShaderP* = nullptr);
        ~Material();

        const std::unordered_map<uint,MaterialComponent*>& getComponents() const;
        const MaterialComponent* getComponent(uint type) const;

        const MaterialComponentReflection* getComponentReflection() const;
        const MaterialComponentRefraction* getComponentRefraction() const;
        const MaterialComponentParallaxOcclusion* getComponentParallaxOcclusion() const;

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

        void addComponentParallaxOcclusion(Texture* texture,float heightScale = 0.1f);
        void addComponentParallaxOcclusion(std::string textureFile,float heightScale = 0.1f);

        const uint id() const;
    
        const bool shadeless() const;
        const float glow() const;
        const float smoothness() const;
        const float metalness() const;
        const float ao() const;
        
        void setF0Color(glm::vec3 c);
        void setF0Color(float r,float g,float b);

        void setMaterialPhysics(MaterialPhysics::Physics);
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
