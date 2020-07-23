#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

class  ShaderProgram;
class  Mesh;
class  MaterialComponent;
class  Texture;
namespace Engine::priv {
    struct DefaultMaterialBindFunctor;
    struct InternalMaterialPublicInterface;
    struct InternalMaterialRequestPublicInterface;
    struct InternalScenePublicInterface;
    struct MaterialLoader;
    class  Renderer;
    class  IRenderingPipeline;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <core/engine/resources/Resource.h>
#include <core/engine/materials/MaterialEnums.h>

class Material final : public Resource {
    friend struct Engine::priv::DefaultMaterialBindFunctor;
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
        std::vector<MaterialComponent*>   m_Components;
        std::function<void(Material*)>    m_CustomBindFunctor   = [](Material*) {};
      //std::function<void(Material*)>    m_CustomUnbindFunctor = [](Material*) {};

        unsigned char                     m_DiffuseModel        = (unsigned char)DiffuseModel::Lambert;
        unsigned char                     m_SpecularModel       = (unsigned char)SpecularModel::GGX;
        bool                              m_Shadeless           = false;
        bool                              m_UpdatedThisFrame    = false;
        Engine::color_vector_4            m_F0Color             = Engine::color_vector_4(10_uc, 10_uc, 10_uc, 255_uc);
        unsigned char                     m_BaseGlow            = 1_uc;
        unsigned char                     m_BaseAO              = 254_uc;
        unsigned char                     m_BaseMetalness       = 1_uc;
        unsigned char                     m_BaseSmoothness      = 64_uc;
        unsigned char                     m_BaseAlpha           = 254_uc;
        std::uint32_t                     m_ID                  = 0U;

        MaterialComponent* internalAddComponentGeneric(MaterialComponentType::Type type, Texture* texture, Texture* mask = nullptr, Texture* cubemap = nullptr);
        void internalUpdateGlobalMaterialPool(bool addToDatabase);
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

        void setCustomBindFunctor(std::function<void(Material*)> functor) noexcept { m_CustomBindFunctor = functor; }

        Material(const Material&)                      = delete;
        Material& operator=(const Material&)           = delete;
        Material(Material&& other) noexcept            = delete;
        Material& operator=(Material&& other) noexcept = delete;

        MaterialComponent& getComponent(unsigned int index);

        MaterialComponent& addComponent(MaterialComponentType::Type type, const std::string& textureFile = "", const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialComponent& addComponentDiffuse(const std::string& textureFile);
        MaterialComponent& addComponentNormal(const std::string& textureFile);
        MaterialComponent& addComponentGlow(const std::string& textureFile);
        MaterialComponent& addComponentSpecular(const std::string& textureFile);
        MaterialComponent& addComponentAO(const std::string& textureFile, unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentMetalness(const std::string& textureFile, unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentSmoothness(const std::string& textureFile, unsigned char baseValue = 255_uc);
        MaterialComponent& addComponentReflection(const std::string& cubeMapName, const std::string& mapFile, float mixFactor = 1.0f);
        MaterialComponent& addComponentRefraction(const std::string& cubeMapName, const std::string& mapFile, float refractiveIndex = 1.0f, float mixFactor = 1.0f);
        MaterialComponent& addComponentParallaxOcclusion(const std::string& textureFile, float heightScale = 0.1f);

        inline CONSTEXPR bool shadeless() const noexcept { return m_Shadeless; }
        inline CONSTEXPR const Engine::color_vector_4& f0() const noexcept { return m_F0Color; }
        inline CONSTEXPR unsigned char glow() const noexcept { return m_BaseGlow; }
        inline CONSTEXPR std::uint32_t id() const noexcept { return m_ID; }
        inline CONSTEXPR unsigned char diffuseModel() const noexcept { return m_DiffuseModel; }
        inline CONSTEXPR unsigned char specularModel() const noexcept { return m_SpecularModel; }
        inline CONSTEXPR unsigned char ao() const noexcept { return m_BaseAO; }
        inline CONSTEXPR unsigned char metalness() const noexcept { return m_BaseMetalness; }
        inline CONSTEXPR unsigned char smoothness() const noexcept { return m_BaseSmoothness; }
        inline CONSTEXPR unsigned char alpha() const noexcept { return m_BaseAlpha; }
        
        void setF0Color(const Engine::color_vector_4& f0Color);
        void setF0Color(unsigned char r, unsigned char g, unsigned char b);

        void setMaterialPhysics(MaterialPhysics materialPhysics);
        void setShadeless(bool shadeless);
        void setGlow(unsigned char glow);
        void setSmoothness(unsigned char smoothness);
        void setAO(unsigned char ao);
        void setMetalness(unsigned char metalness);
        void setAlpha(unsigned char alpha);
    
        void setSpecularModel(SpecularModel specularModel);
        void setDiffuseModel(DiffuseModel diffuseModel);

        void update(const float dt);
};
#endif
