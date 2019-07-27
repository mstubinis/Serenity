#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

#include <core/engine/BindableResource.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <unordered_map>

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialComponent.h>

class  ShaderP;
class  Mesh;

namespace Engine{
    namespace epriv{
        struct DefaultMaterialBindFunctor;
        struct DefaultMaterialUnbindFunctor;
        class  RenderManager;
    };
};

class Material final: public BindableResource{
    friend struct Engine::epriv::DefaultMaterialBindFunctor;
    friend struct Engine::epriv::DefaultMaterialUnbindFunctor;
    friend class  Engine::epriv::RenderManager;
    public:
        static Material*                  Checkers; //loaded in renderer
    private:
        static std::vector<glm::vec4>     m_MaterialProperities;
        std::vector<MaterialComponent*>   m_Components;
        uint                              m_DiffuseModel;
        uint                              m_SpecularModel;
        bool                              m_Shadeless;
        glm::vec3                         m_F0Color;
        float                             m_BaseGlow;
        float                             m_BaseAO;
        float                             m_BaseMetalness;
        float                             m_BaseSmoothness;
        uint                              m_ID;

        void internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture);
        void internalUpdateGlobalMaterialPool(const bool& addToDatabase);
        void internalInit(Texture* diffuse, Texture* normal, Texture* glow, Texture* specular);

    public:
        Material(const std::string& name, const std::string& diffuse, const std::string& normal="", const std::string& glow="", const std::string& specular="");
        Material(const std::string& name, Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr);
        ~Material();

        const MaterialComponent* getComponent(const MaterialComponentType::Type& type) const;

        //const MaterialComponentReflection* getComponentReflection() const;
        //const MaterialComponentRefraction* getComponentRefraction() const;
        //const MaterialComponentParallaxOcclusion* getComponentParallaxOcclusion() const;

        void addComponentDiffuse(Texture* texture);
        void addComponentDiffuse(const std::string& textureFile);

        void addComponentNormal(Texture* texture);
        void addComponentNormal(const std::string& textureFile);

        void addComponentGlow(Texture* texture);
        void addComponentGlow(const std::string& textureFile);

        void addComponentSpecular(Texture* texture);
        void addComponentSpecular(const std::string& textureFile);

        void addComponentAO(Texture* texture, const float baseValue = 1.0f);
        void addComponentAO(const std::string& textureFile, const float baseValue = 1.0f);

        void addComponentMetalness(Texture* texture, const float baseValue = 1.0f);
        void addComponentMetalness(const std::string& textureFile, const float baseValue = 1.0f);

        void addComponentSmoothness(Texture* texture, const float baseValue = 1.0f);
        void addComponentSmoothness(const std::string& textureFile, const float baseValue = 1.0f);

        void addComponentReflection(Texture* cubeMap,Texture* map, const float mixFactor = 1.0f);
        void addComponentReflection(const std::string& cubeMapName, const std::string& mapFile, const float mixFactor = 1.0f);
        void addComponentReflection(const std::string cubeMapTextureFiles[], const std::string& mapFile, const float mixFactor = 1.0f);

        void addComponentRefraction(Texture* cubeMap,Texture* map, const float refractiveIndex = 1.0f, const float mixFactor = 1.0f);
        void addComponentRefraction(const std::string& cubeMapName, const std::string& mapFile, const float refractiveIndex = 1.0f, const float mixFactor = 1.0f);
        void addComponentRefraction(const std::string cubeMapTextureFiles[], const std::string& mapFile, const float refractiveIndex = 1.0f, const float mixFactor = 1.0f);

        void addComponentParallaxOcclusion(Texture* texture,float heightScale = 0.1f);
        void addComponentParallaxOcclusion(const std::string& textureFile, const float heightScale = 0.1f);

        const uint& id() const;
    
        const bool& shadeless() const;
        const glm::vec3& f0() const;
        const float& glow() const;
        const float& smoothness() const;
        const float& metalness() const;
        const float& ao() const;
        
        void setF0Color(const glm::vec3& f0Color);
        void setF0Color(const float& r, const float& g, const float& b);

        void setMaterialPhysics(const MaterialPhysics::Physics& materialPhysics);
        void setShadeless(const bool& shadeless);
        void setGlow(const float& glow);
        void setSmoothness(const float& smoothness);
        void setAO(const float& ao);
        void setMetalness(const float& metalness);
    
        const uint& specularModel() const;
        void setSpecularModel(const SpecularModel::Model& specularModel);
        const uint& diffuseModel() const;    
        void setDiffuseModel(const DiffuseModel::Model& diffuseModel);

        void bind();
        void unbind();

        void load();
        void unload();
};
#endif
