#pragma once
#ifndef ENGINE_MATERIAL_H
#define ENGINE_MATERIAL_H

class  ShaderProgram;
class  Mesh;
class  MaterialComponent;
class  Texture;
namespace Engine::priv {
    struct PublicMaterial;
    struct PublicScene;
    class  MaterialLoader;
    class  RenderModule;
    class  IRenderingPipeline;
};
#include <serenity/resources/Resource.h>
#include <serenity/resources/Handle.h>
#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/types/Types.h>
#include <serenity/system/TypeDefs.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct MaterialDefaultPhysicsProperty final {
    uint8_t r          = 0;
    uint8_t g          = 0;
    uint8_t b          = 0;
    uint8_t smoothness = 0;
    uint8_t metalness  = 0;
};

class Material final : public Resource<Material> {
    friend struct Engine::priv::PublicScene;
    friend struct Engine::priv::PublicMaterial;
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

        Engine::color_vector_4            m_F0Color             = Engine::color_vector_4(10_uc, 10_uc, 10_uc, 255_uc); //basically a uint32_t
        MaterialID                        m_ID                  = 0U; //uint16_t
        DiffuseModel                      m_DiffuseModel        = DiffuseModel::Lambert; //uint8_t
        SpecularModel                     m_SpecularModel       = SpecularModel::Cook_Torrance; //uint8_t
        uint8_t                           m_BaseGlow            = 1_uc;
        uint8_t                           m_BaseMetalness       = 1_uc;
        uint8_t                           m_BaseSmoothness      = 64_uc;
        uint8_t                           m_BaseAO              = 254_uc;
        uint8_t                           m_BaseAlpha           = 254_uc;
        bool                              m_Shadeless           = false;
        bool                              m_UpdatedThisFrame    = false;
    private:
        MaterialComponent* internal_add_component_generic(MaterialComponentType type, Handle texture, Handle mask = Handle{}, Handle cubemap = Handle{});
        void internal_update_global_material_pool(bool addToDatabase) noexcept;
    public:
        Material();
        Material(
            std::string_view name,
            std::string_view diffuse,
            std::string_view normal     = "",
            std::string_view glow       = "",
            std::string_view specular   = "",
            std::string_view ao         = "",
            std::string_view metalness  = "",
            std::string_view smoothness = ""
        );
        Material(
            std::string_view name,
            Handle diffuse,
            Handle normal     = Handle{},
            Handle glow       = Handle{},
            Handle specular   = Handle{},
            Handle ao         = Handle{},
            Handle metalness  = Handle{},
            Handle smoothness = Handle{}
        );

        Material(const Material&)                 = delete;
        Material& operator=(const Material&)      = delete;
        Material(Material&&) noexcept;
        Material& operator=(Material&&) noexcept;
        ~Material();

        inline void setCustomBindFunctor(const bind_fp& functor) noexcept { m_CustomBindFunctor = functor; }
        inline void setCustomBindFunctor(bind_fp&& functor) noexcept { m_CustomBindFunctor = std::move(functor); }

        inline const std::vector<MaterialComponent>& getComponents() const noexcept { return m_Components; }
        [[nodiscard]] inline MaterialComponent& getComponent(uint32_t index) { return m_Components[index]; }

        MaterialComponent& addComponent(MaterialComponentType type, std::string_view textureFile = "", std::string_view maskFile = "", std::string_view cubemapFile = "");
        MaterialComponent& addComponentDiffuse(std::string_view textureFile);
        MaterialComponent& addComponentNormal(std::string_view textureFile);
        MaterialComponent& addComponentGlow(std::string_view textureFile);
        MaterialComponent& addComponentSpecular(std::string_view textureFile);
        MaterialComponent& addComponentMetalness(std::string_view textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentSmoothness(std::string_view textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentAO(std::string_view textureFile, uint8_t baseValue = 255_uc);
        MaterialComponent& addComponentReflection(std::string_view cubeMapName, const std::string& mapFile, float mixFactor = 1.0f);
        MaterialComponent& addComponentRefraction(std::string_view cubeMapName, const std::string& mapFile, float refractiveIndex = 1.0f, float mixFactor = 1.0f);
        MaterialComponent& addComponentParallaxOcclusion(std::string_view textureFile, float heightScale = 0.1f);

        [[nodiscard]] inline bool getShadeless() const noexcept { return m_Shadeless; }
        [[nodiscard]] inline const Engine::color_vector_4& getF0() const noexcept { return m_F0Color; }
        [[nodiscard]] inline uint8_t getGlow() const noexcept { return m_BaseGlow; }
        [[nodiscard]] inline MaterialID getID() const noexcept { return m_ID; }
        [[nodiscard]] inline DiffuseModel getDiffuseModel() const noexcept { return m_DiffuseModel; }
        [[nodiscard]] inline SpecularModel getSpecularModel() const noexcept { return m_SpecularModel; }
        [[nodiscard]] inline uint8_t getMetalness() const noexcept { return m_BaseMetalness; }
        [[nodiscard]] inline uint8_t getSmoothness() const noexcept { return m_BaseSmoothness; }
        [[nodiscard]] inline uint8_t getAO() const noexcept { return m_BaseAO; }
        [[nodiscard]] inline uint8_t getAlpha() const noexcept { return m_BaseAlpha; }
        
        void setF0Color(uint8_t r, uint8_t g, uint8_t b) noexcept;
        inline void setF0Color(const Engine::color_vector_4& f0Color) noexcept { Material::setF0Color(f0Color.rc(), f0Color.gc(), f0Color.bc()); }

        void setMaterialPhysics(MaterialPhysics);
        void setShadeless(bool shadeless);
        void setGlow(uint8_t glow);
        void setMetalness(uint8_t metalness);
        void setSmoothness(uint8_t smoothness);
        void setAO(uint8_t ao);
        void setAlpha(uint8_t alpha);
    
        void setSpecularModel(SpecularModel);
        void setDiffuseModel(DiffuseModel);

        void update(const float dt);
};

#endif
