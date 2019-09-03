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

class  ShaderProgram;
class  Mesh;

namespace Engine{
    namespace epriv{
        struct DefaultMaterialBindFunctor;
        struct DefaultMaterialUnbindFunctor;
        struct InternalMaterialPublicInterface;
        struct InternalMaterialRequestPublicInterface;
        struct MaterialLoader;
        class  RenderManager;
    };
};

class Material final: public BindableResource{
    friend struct Engine::epriv::DefaultMaterialBindFunctor;
    friend struct Engine::epriv::DefaultMaterialUnbindFunctor;
    friend struct Engine::epriv::InternalMaterialRequestPublicInterface;
    friend struct Engine::epriv::InternalMaterialPublicInterface;
    friend struct Engine::epriv::MaterialLoader;
    friend class  Engine::epriv::RenderManager;
    public:
        static Material                  *Checkers, *WhiteShadeless; //loaded in renderer
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
        float                             m_BaseAlpha;
        uint                              m_ID;

        MaterialComponent* internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        void internalUpdateGlobalMaterialPool(const bool& addToDatabase);
        Material();
    public:
        Material(const std::string& name, const std::string& diffuse, const std::string& normal="", const std::string& glow="", const std::string& specular="");
        Material(const std::string& name, Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr,Texture* specular = nullptr);
        ~Material();

        MaterialComponent& getComponent(const uint& index);

        MaterialComponent& addComponent(const MaterialComponentType::Type& type, const std::string& textureFile = "", const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialComponent& addComponentDiffuse(const std::string& textureFile);
        MaterialComponent& addComponentNormal(const std::string& textureFile);
        MaterialComponent& addComponentGlow(const std::string& textureFile);
        MaterialComponent& addComponentSpecular(const std::string& textureFile);
        MaterialComponent& addComponentAO(const std::string& textureFile, const float baseValue = 1.0f);
        MaterialComponent& addComponentMetalness(const std::string& textureFile, const float baseValue = 1.0f);
        MaterialComponent& addComponentSmoothness(const std::string& textureFile, const float baseValue = 1.0f);
        MaterialComponent& addComponentReflection(const std::string& cubeMapName, const std::string& mapFile, const float mixFactor = 1.0f);
        MaterialComponent& addComponentRefraction(const std::string& cubeMapName, const std::string& mapFile, const float refractiveIndex = 1.0f, const float mixFactor = 1.0f);
        MaterialComponent& addComponentParallaxOcclusion(const std::string& textureFile, const float heightScale = 0.1f);

        const uint& id() const;
    
        const bool& shadeless() const;
        const glm::vec3& f0() const;
        const float& glow() const;
        const float& smoothness() const;
        const float& metalness() const;
        const float& ao() const;
        const float& alpha() const;
        
        void setF0Color(const glm::vec3& f0Color);
        void setF0Color(const float& r, const float& g, const float& b);

        void setMaterialPhysics(const MaterialPhysics::Physics& materialPhysics);
        void setShadeless(const bool& shadeless);
        void setGlow(const float& glow);
        void setSmoothness(const float& smoothness);
        void setAO(const float& ao);
        void setMetalness(const float& metalness);
        void setAlpha(const float& alpha);
    
        const uint& specularModel() const;
        void setSpecularModel(const SpecularModel::Model& specularModel);
        const uint& diffuseModel() const;    
        void setDiffuseModel(const DiffuseModel::Model& diffuseModel);

        void bind();
        void unbind();

        void update(const double& dt);
};
#endif
