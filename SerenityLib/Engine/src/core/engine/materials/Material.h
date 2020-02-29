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
    class  Renderer;
    class  IRenderingPipeline;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/materials/MaterialEnums.h>
#include <functional>

class Material final : public EngineResource{
    friend struct Engine::priv::DefaultMaterialBindFunctor;
    friend struct Engine::priv::DefaultMaterialUnbindFunctor;
    friend struct Engine::priv::InternalScenePublicInterface;
    friend struct Engine::priv::InternalMaterialRequestPublicInterface;
    friend struct Engine::priv::InternalMaterialPublicInterface;
    friend struct Engine::priv::MaterialLoader;
    friend class  Engine::priv::Renderer;
    friend class  Engine::priv::IRenderingPipeline;
    public:
        static Material                  *Checkers, *WhiteShadeless; //loaded in renderer

        static std::vector<glm::vec4>     m_MaterialProperities;
    private:
        std::function<void(Material*)>    m_CustomBindFunctor;

        std::vector<MaterialComponent*>   m_Components;
        unsigned char                     m_DiffuseModel       = DiffuseModel::Lambert;
        unsigned char                     m_SpecularModel      = SpecularModel::GGX;
        bool                              m_Shadeless          = false;
        bool                              m_UpdatedThisFrame   = false;
        Engine::color_vector_4            m_F0Color            = Engine::color_vector_4(10_uc, 10_uc, 10_uc, 255_uc);
        unsigned char                     m_BaseGlow           = 1_uc;
        unsigned char                     m_BaseAO             = 254_uc;
        unsigned char                     m_BaseMetalness      = 1_uc;
        unsigned char                     m_BaseSmoothness     = 64_uc;
        unsigned char                     m_BaseAlpha          = 254_uc;
        std::uint32_t                     m_ID                 = 0;

        MaterialComponent* internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        void internalUpdateGlobalMaterialPool(const bool addToDatabase);
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


        template<typename T>
        void setCustomBindFunctor(const T& functor) {
            m_CustomBindFunctor = std::bind<void>(std::move(functor), std::placeholders::_1);
        }


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
        MaterialComponent& addComponentAO(const std::string& textureFile, const unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentMetalness(const std::string& textureFile, const unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentSmoothness(const std::string& textureFile, const unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentReflection(const std::string& cubeMapName, const std::string& mapFile, const float mixFactor = 1.0f);
        MaterialComponent& addComponentRefraction(const std::string& cubeMapName, const std::string& mapFile, const float refractiveIndex = 1.0f, const float mixFactor = 1.0f);
        MaterialComponent& addComponentParallaxOcclusion(const std::string& textureFile, const float heightScale = 0.1f);

        const std::uint32_t id() const;
    
        const bool shadeless() const;
        const Engine::color_vector_4& f0() const;
        const unsigned char glow() const;
        const unsigned char smoothness() const;
        const unsigned char metalness() const;
        const unsigned char ao() const;
        const unsigned char alpha() const;
        
        void setF0Color(const Engine::color_vector_4& f0Color);
        void setF0Color(const unsigned char r, const unsigned char g, const unsigned char b);

        void setMaterialPhysics(const MaterialPhysics::Physics materialPhysics);
        void setShadeless(const bool shadeless);
        void setGlow(const unsigned char glow);
        void setSmoothness(const unsigned char smoothness);
        void setAO(const unsigned char ao);
        void setMetalness(const unsigned char metalness);
        void setAlpha(const unsigned char alpha);
    
        const unsigned char specularModel() const;
        void setSpecularModel(const SpecularModel::Model specularModel);
        const unsigned char diffuseModel() const;
        void setDiffuseModel(const DiffuseModel::Model diffuseModel);

        void update(const float dt);
};
#endif
