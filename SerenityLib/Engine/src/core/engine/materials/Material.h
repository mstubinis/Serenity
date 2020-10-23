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
#include <core/engine/resources/Resource.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/materials/MaterialComponent.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct MaterialDefaultPhysicsProperty final {
    uint8_t r          = 0_uc;
    uint8_t g          = 0_uc;
    uint8_t b          = 0_uc;
    uint8_t smoothness = 0_uc;
    uint8_t metalness  = 0_uc;
};

class Material final : public Resource {
    friend struct Engine::priv::InternalScenePublicInterface;
    friend struct Engine::priv::InternalMaterialRequestPublicInterface;
    friend struct Engine::priv::InternalMaterialPublicInterface;
    friend class  Engine::priv::MaterialLoader;
    friend class  Engine::priv::RenderModule;
    friend class  Engine::priv::IRenderingPipeline;

    using bind_fp    = void(*)(Material*);
  //using unbind_fp  = void(*)(Material*);
    using MaterialID = uint16_t;

    public:
        static Handle Checkers, WhiteShadeless; //loaded in renderer
        static std::vector<glm::vec4>     m_MaterialProperities;
    private:
        std::vector<MaterialComponent>    m_Components;
        bind_fp                           m_CustomBindFunctor   = [](Material*) {};
      //unbind_fp                         m_CustomUnbindFunctor = [](Material*) {};

        Engine::color_vector_4            m_F0Color             = Engine::color_vector_4(10_uc, 10_uc, 10_uc, 255_uc);
        MaterialID                        m_ID                  = 0U;
        DiffuseModel                      m_DiffuseModel        = DiffuseModel::Lambert;
        SpecularModel                     m_SpecularModel       = SpecularModel::Cook_Torrance;
        uint8_t                           m_BaseGlow            = 1_uc;
        uint8_t                           m_BaseAO              = 254_uc;
        uint8_t                           m_BaseMetalness       = 1_uc;
        uint8_t                           m_BaseSmoothness      = 64_uc;
        uint8_t                           m_BaseAlpha           = 254_uc;
        bool                              m_Shadeless           = false;
        bool                              m_UpdatedThisFrame    = false;
    private:
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

        inline const std::vector<MaterialComponent>& getComponents() const noexcept { return m_Components; }
        inline MaterialComponent& getComponent(uint32_t index) { return m_Components[index]; }

        MaterialComponent& addComponent(MaterialComponentType type, const std::string& textureFile = "", const std::string& maskFile = "", const std::string& cubemapFile = "");
        MaterialComponent& addComponentDiffuse(const std::string& textureFile);
        MaterialComponent& addComponentNormal(const std::string& textureFile);
        MaterialComponent& addComponentGlow(const std::string& textureFile);
        MaterialComponent& addComponentSpecular(const std::string& textureFile);
        MaterialComponent& addComponentAO(const std::string& textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentMetalness(const std::string& textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentSmoothness(const std::string& textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentReflection(const std::string& cubeMapName, const std::string& mapFile, float mixFactor = 1.0f);
        MaterialComponent& addComponentRefraction(const std::string& cubeMapName, const std::string& mapFile, float refractiveIndex = 1.0f, float mixFactor = 1.0f);
        MaterialComponent& addComponentParallaxOcclusion(const std::string& textureFile, float heightScale = 0.1f);

        inline bool shadeless() const noexcept { return m_Shadeless; }
        inline const Engine::color_vector_4& f0() const noexcept { return m_F0Color; }
        inline uint8_t glow() const noexcept { return m_BaseGlow; }
        inline MaterialID id() const noexcept { return m_ID; }
        inline DiffuseModel diffuseModel() const noexcept { return m_DiffuseModel; }
        inline SpecularModel specularModel() const noexcept { return m_SpecularModel; }
        inline uint8_t ao() const noexcept { return m_BaseAO; }
        inline uint8_t metalness() const noexcept { return m_BaseMetalness; }
        inline uint8_t smoothness() const noexcept { return m_BaseSmoothness; }
        inline uint8_t alpha() const noexcept { return m_BaseAlpha; }
        
        void setF0Color(const Engine::color_vector_4& f0Color);
        void setF0Color(uint8_t r, uint8_t g, uint8_t b);

        void setMaterialPhysics(MaterialPhysics materialPhysics);
        void setShadeless(bool shadeless);
        void setGlow(uint8_t glow);
        void setSmoothness(uint8_t smoothness);
        void setAO(uint8_t ao);
        void setMetalness(uint8_t metalness);
        void setAlpha(uint8_t alpha);
    
        void setSpecularModel(SpecularModel specularModel);
        void setDiffuseModel(DiffuseModel diffuseModel);

        void update(const float dt);
};

#endif
