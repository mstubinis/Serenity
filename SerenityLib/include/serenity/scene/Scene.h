#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

class  Scene;
class  Camera;
class  SunLight;
class  DirectionalLight;
class  PointLight;
class  SpotLight;
class  RodLight;
class  ProjectionLight;
class  ShaderProgram;
class  Viewport;
class  Skybox;
class  ModelInstance;
class  ComponentModel;
class  Entity;
struct SceneOptions;

class ParticleEmitter;
class ParticleEmissionProperties;
class Particle;

namespace Engine::priv {
    class  ECS;
    class  RenderGraph;
    class  ResourceManager;
    struct PublicScene;
    class  GBuffer;
    class  RenderModule;
    class  EngineCore;
};
#include <serenity/ecs/ECS.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/scene/Viewport.h>
#include <serenity/events/Observer.h>
#include <serenity/resources/Handle.h>
#include <serenity/scene/LightsModule.h>
#include <serenity/renderer/RenderGraph.h>

class Scene: public Observer {
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::PublicScene;
    friend class  Engine::priv::EngineCore;
    using UpdateFP             = void(*)(Scene*, const float);
    private:
        mutable std::vector<Viewport>               m_Viewports;
        mutable std::vector<Camera*>                m_Cameras;
        mutable Engine::priv::RenderGraphContainer  m_RenderGraphs;

        mutable Engine::priv::LightsModule          m_LightsModule;

        UpdateFP                                    m_OnUpdateFunctor     = [](Scene*, const float) {};
        std::string                                 m_Name;
        uint32_t                                    m_ID                  = 0;
        glm::vec3                                   m_GI                  = glm::vec3{ 1.0f };
        bool                                        m_SkipRenderThisFrame = false;

        Entity                                      m_Sun;
        Skybox*                                     m_Skybox              = nullptr;

        class impl; std::unique_ptr<impl>           m_i                   = nullptr;

        void preUpdate(const float dt);
        void postUpdate(const float dt);
    public:
        Scene(std::string_view name);
        Scene(std::string_view name, const SceneOptions&);
        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = delete;
        Scene& operator=(Scene&&) noexcept = delete;
        virtual ~Scene();

        inline void setName(std::string_view name) noexcept { m_Name = name; }
        [[nodiscard]] inline constexpr const std::string& name() const noexcept { return m_Name; }

        template<class LIGHT, typename ... ARGS> 
        [[nodiscard]] inline Engine::view_ptr<LIGHT> createLight(ARGS&& ... args) {
            return m_LightsModule.createLight<LIGHT>(this, std::forward<ARGS>(args)...);
        }
        template<class LIGHT> 
        inline void deleteLight(LIGHT* light) noexcept { 
            m_LightsModule.deleteLight(light); 
        }

        void clearAllEntities() noexcept;
        void update(const float dt);
        virtual void render() {}
        virtual void onEvent(const Event&);
        virtual void onResize(unsigned int width, unsigned int height) {}

        inline void setOnUpdateFunctor(const UpdateFP& functor) noexcept { m_OnUpdateFunctor = functor; }
        inline void setOnUpdateFunctor(UpdateFP&& functor) noexcept { m_OnUpdateFunctor = std::move(functor); }

        [[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
        [[nodiscard]] inline uint32_t numViewports() const noexcept { return (uint32_t)m_Viewports.size(); }


        [[nodiscard]] Entity createEntity();
        void removeEntity(Entity);

        [[nodiscard]] size_t getNumLights() const noexcept;

        [[nodiscard]] Viewport& getMainViewport();
        Viewport& addViewport(float x, float y, float width, float height, Camera& camera);

        [[nodiscard]] ParticleEmitter* addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity* parent = nullptr);


        [[nodiscard]] Camera* getActiveCamera() const;
        void setActiveCamera(Camera&);

        void addCamera(Camera&);
        [[nodiscard]] Camera* addCamera(float left, float right, float top, float bottom, float Near, float Far);
        [[nodiscard]] Camera* addCamera(float angle, float aspectRatio, float Near, float Far);

        [[nodiscard]] const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(float r, float g, float b, float a);
        void setBackgroundColor(const glm::vec4& backgroundColor);

        [[nodiscard]] inline constexpr const glm::vec3& getGlobalIllumination() const noexcept { return m_GI; }
        void setGlobalIllumination(float global, float diffuse, float specular);
        void setGlobalIllumination(const glm::vec3& globalIllumination);

        inline void setGodRaysSun(Entity sun) noexcept { m_Sun = sun; }
        [[nodiscard]] inline constexpr Entity getGodRaysSun() const noexcept { return m_Sun; }
        [[nodiscard]] inline constexpr Skybox* skybox() const noexcept { return m_Skybox; }
        inline void setSkybox(Skybox* s) noexcept { m_Skybox = s; }

        void centerSceneToObject(Entity);
};
namespace Engine::priv {
    struct PublicScene final {
        friend class Scene;
        friend class Engine::priv::RenderGraph;

        static std::vector<Particle>&            GetParticles(const Scene& scene);
        static std::vector<Entity>&              GetEntities(const Scene& scene);
        static std::vector<Viewport>&            GetViewports(const Scene& scene);
        static std::vector<Camera*>&             GetCameras(const Scene& scene);

        template<class LIGHT>
        [[nodiscard]] static inline const Engine::priv::LightContainer<LIGHT>& GetLights(const Scene& scene) noexcept {
            return scene.m_LightsModule.getLights<LIGHT>(); 
        }
        template<class LIGHT>
        static inline bool SetLightShadowCaster(const Scene& scene, LIGHT& light, bool isShadowCaster) noexcept {
            return scene.m_LightsModule.setShadowCaster(&light, isShadowCaster);
        }

        static void                       UpdateMaterials(Scene&, const float dt);

        static void                       RenderGeometryOpaque(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparent(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparentTrianglesSorted(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardOpaque(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardTransparent(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardTransparentTrianglesSorted(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardParticles(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderDecals(RenderModule&, Scene&, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderParticles(RenderModule&, Scene&, Viewport&, Camera&, Handle program);

        static void                       AddModelInstanceToPipeline(Scene&, ModelInstance&, RenderStage);
        static void                       RemoveModelInstanceFromPipeline(Scene&, ModelInstance&, RenderStage);
        [[nodiscard]] static Engine::priv::ECS& GetECS(Scene&);
        static void                       CleanECS(Scene&, Entity);
        static void                       SkipRenderThisFrame(Scene&, bool isSkip);
        [[nodiscard]] static bool         IsSkipRenderThisFrame(Scene&);
        [[nodiscard]] static bool         HasItemsToRender(Scene&);
    };
};

#endif