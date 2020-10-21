#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

class  ShaderProgram;
class  Mesh;
class  MaterialComponent;
class  Texture;
namespace Engine::priv {
    struct InternalMaterialPublicInterface;
    struct InternalMaterialRequestPublicInterface;
    struct InternalScenePublicInterface;
    class  MaterialLoader;
    class  RenderModule;
    class  IRenderingPipeline;
};

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <core/engine/resources/Resource.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialComponent.h>

struct MaterialDefaultPhysicsProperty final {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t smoothness;
    std::uint8_t metalness;
};

class Material final : public Resource {
    friend struct Engine::priv::InternalScenePublicInterface;
    friend struct Engine::priv::InternalMaterialRequestPublicInterface;
    friend struct Engine::priv::InternalMaterialPublicInterface;
    friend class  Engine::priv::MaterialLoader;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;

    using bind_fp   = void(*)(Material*);
  //using unbind_fp = void(*)(Material*);

    using MaterialID = uint16_t;

    public:
        static Handle Checkers, WhiteShadeless; //loaded in renderer
        static std::vector<glm::vec4>     m_MaterialProperities;
    private:
        std::vector<MaterialComponent>    m_Components;
        bind_fp                           m_CustomBindFunctor   = [](Material*) {};
      //unbind_fp                         m_CustomUnbindFunctor = [](Material*) {};

        DiffuseModel                      m_DiffuseModel        = DiffuseModel::Lambert;
        SpecularModel                     m_SpecularModel       = SpecularModel::Cook_Torrance;
        bool                              m_Shadeless           = false;
        bool                              m_UpdatedThisFrame    = false;
        Engine::color_vector_4            m_F0Color             = Engine::color_vector_4(10_uc, 10_uc, 10_uc, 255_uc);
        unsigned char                     m_BaseGlow            = 1_uc;
        unsigned char                     m_BaseAO              = 254_uc;
        unsigned char                     m_BaseMetalness       = 1_uc;
        unsigned char                     m_BaseSmoothness      = 64_uc;
        unsigned char                     m_BaseAlpha           = 254_uc;
        MaterialID                        m_ID                  = 0U;

        MaterialComponent* internal_add_component_generic(MaterialComponentType type, Handle texture, Handle mask = Handle{}, Handle cubemap = {});
        void internal_update_global_material_pool(bool addToDatabase);
    public:
        Material();
        Material(
            const std::string& name,
            const std::string& diffuse,
            const std::string& normal     = "",
            const std::string& glow       = "",
            const std::string& specular   = "",
            const std::string& ao         = "",
            const std::string& metalness  = "",
            const std::string& smoothness = ""
        );
        Material(
            const std::string& name,
            Handle diffuse,
            Handle normal     = Handle{},
            Handle glow       = Handle{},
            Handle specular   = Handle{},
            Handle ao         = Handle{},
            Handle metalness  = Handle{},
            Handle smoothness = Handle{}
        );

        Material(const Material& other)                 = delete;
        Material& operator=(const Material& other)      = delete;
        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;
        ~Material();

        inline void setCustomBindFunctor(const bind_fp& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomBindFunctor(bind_fp&& functor) noexcept { m_CustomBindFunctor = std::move(functor); }

        inline CONSTEXPR const std::vector<MaterialComponent>& getComponents() const noexcept { return m_Components; }
        inline CONSTEXPR MaterialComponent& getComponent(unsigned int index) { return m_Components[index]; }

        MaterialComponent& addComponent(MaterialComponentType type, const std::string& textureFile = "", const std::string& maskFile = "", const std::string& cubemapFile = "");
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
        inline CONSTEXPR MaterialID id() const noexcept { return m_ID; }
        inline CONSTEXPR DiffuseModel diffuseModel() const noexcept { return m_DiffuseModel; }
        inline CONSTEXPR SpecularModel specularModel() const noexcept { return m_SpecularModel; }
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
