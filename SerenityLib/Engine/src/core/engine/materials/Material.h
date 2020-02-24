#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

class  ShaderProgram;
class  Mesh;
class  MaterialComponent;
class  Texture;
namespace Engine::priv {
    struct DefaultMaterialBindFunctor;
    struct DefaultMaterialUnbindFunctor;
    struct InternalMaterialPublicInterface;
    struct InternalMaterialRequestPublicInterface;
    struct InternalScenePublicInterface;
    struct MaterialLoader;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include <core/engine/BindableResource.h>
#include <core/engine/materials/MaterialEnums.h>

class Material final: public BindableResource{
    friend struct Engine::priv::DefaultMaterialBindFunctor;
    friend struct Engine::priv::DefaultMaterialUnbindFunctor;
    friend struct Engine::priv::InternalScenePublicInterface;
    friend struct Engine::priv::InternalMaterialRequestPublicInterface;
    friend struct Engine::priv::InternalMaterialPublicInterface;
    friend struct Engine::priv::MaterialLoader;
    public:
        static Material                  *Checkers, *WhiteShadeless; //loaded in renderer

        static std::vector<glm::vec4>     m_MaterialProperities;
    private:
        std::vector<MaterialComponent*>   m_Components;
        unsigned int                      m_DiffuseModel;
        unsigned int                      m_SpecularModel;
        bool                              m_Shadeless;
        bool                              m_UpdatedThisFrame;
        glm::vec3                         m_F0Color;
        float                             m_BaseGlow;
        float                             m_BaseAO;
        float                             m_BaseMetalness;
        float                             m_BaseSmoothness;
        float                             m_BaseAlpha;
        size_t                            m_ID;

        MaterialComponent* internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        void internalUpdateGlobalMaterialPool(const bool& addToDatabase);
        Material();
    public:
        Material(
            const std::string& name,
            const std::string& diffuse,
            const std::string& normal = "",
            const std::string& glow = "",
            const std::string& specular = "",
            const std::string& ao = "",
            const std::string& metalness = "",
            const std::string& smoothness = ""
        );
        Material(
            const std::string& name,
            Texture* diffuse,
            Texture* normal = nullptr,
            Texture* glow = nullptr,
            Texture* specular = nullptr,
            Texture* ao = nullptr,
            Texture* metalness = nullptr,
            Texture* smoothness = nullptr
        );
        ~Material();

        Material(const Material&)                      = delete;
        Material& operator=(const Material&)           = delete;
        Material(Material&& other) noexcept            = delete;
        Material& operator=(Material&& other) noexcept = delete;

        MaterialComponent& getComponent(const unsigned int& index);

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

        const size_t& id() const;
    
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
    
        const unsigned int& specularModel() const;
        void setSpecularModel(const SpecularModel::Model& specularModel);
        const unsigned int& diffuseModel() const;
        void setDiffuseModel(const DiffuseModel::Model& diffuseModel);

        void bind() const override;
        void unbind() const override;

        void update(const float& dt);
};
#endif
