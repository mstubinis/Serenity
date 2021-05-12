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
class  SystemSceneUpdate;
class  SystemSceneChanging;
class  SystemAddRigidBodies;
class  SystemRemoveRigidBodies;

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
#include <serenity/renderer/particles/ParticleSystem.h>

class Scene: public Observer {
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::PublicScene;
    friend class  Engine::priv::EngineCore;
    friend class  SystemSceneUpdate;
    friend class  SystemSceneChanging;
    friend class  SystemAddRigidBodies;
    friend class  SystemRemoveRigidBodies;
    using UpdateFP = void(*)(Scene*, const float);
    private:
        Engine::priv::ECS                           m_ECS;
        mutable Engine::priv::ParticleSystem        m_ParticleSystem;
        mutable std::vector<Viewport>               m_Viewports;
        mutable std::vector<Camera*>                m_Cameras;
        mutable Engine::priv::RenderGraphContainer  m_RenderGraphs;
        mutable Engine::priv::LightsModule          m_LightsModule;
        UpdateFP                                    m_OnUpdateFunctor     = [](Scene*, const float) {};
        glm::vec3                                   m_GI                  = glm::vec3{ 1.0f };
        glm::vec3                                   m_AmbientColor        = glm::vec3{ 0.05f, 0.05f, 0.05f };
        std::string                                 m_Name;
        Skybox*                                     m_Skybox              = nullptr;
        uint32_t                                    m_ID                  = 0;
        Entity                                      m_Sun;
        bool                                        m_SkipRenderThisFrame = false;
        bool                                        m_WasJustSwappedTo    = false;

        void internal_register_components();
        void internal_register_systems();
        void preUpdate(const float dt);
        void postUpdate(const float dt);
    public:
        Scene(uint32_t id, std::string_view name);
        Scene(uint32_t id, std::string_view name, const SceneOptions&);
        Scene(const Scene&)                = delete;
        Scene& operator=(const Scene&)     = delete;
        Scene(Scene&&) noexcept            = delete;
        Scene& operator=(Scene&&) noexcept = delete;
        virtual ~Scene();


        template<class COMPONENT> inline void registerComponent() { m_ECS.registerComponent<COMPONENT>(); }
        

        template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
        inline void registerSystem(ARG_TUPLE&& argTuple = ARG_TUPLE{}) {
            m_ECS.registerSystem<SYSTEM, ARG_TUPLE, COMPONENTS...>(std::forward<ARG_TUPLE>(argTuple));
        }
        template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
        inline void registerSystemOrdered(uint32_t order, ARG_TUPLE&& argTuple = ARG_TUPLE{}) {
            m_ECS.registerSystemOrdered<SYSTEM, ARG_TUPLE, COMPONENTS...>(order, std::forward<ARG_TUPLE>(argTuple));
        }

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

        [[nodiscard]] inline const glm::vec3& getAmbientColor() const noexcept { return m_AmbientColor; }
        inline void setAmbientColor(const glm::vec3& color) noexcept { Scene::setAmbientColor(color.r, color.g, color.b); }
        inline void setAmbientColor(float r, float g, float b) noexcept { m_AmbientColor = glm::vec3{ r, g, b }; }

        [[nodiscard]] inline constexpr uint32_t id() const noexcept { return m_ID; }
        [[nodiscard]] inline uint32_t numViewports() const noexcept { return (uint32_t)m_Viewports.size(); }


        [[nodiscard]] Entity createEntity();
        void removeEntity(Entity);

        [[nodiscard]] size_t getNumLights() const noexcept;

        [[nodiscard]] Viewport& getMainViewport();
        Viewport& addViewport(float x, float y, float width, float height, Camera& camera);

        [[nodiscard]] ParticleEmitter* addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity parent = Entity{});


        [[nodiscard]] Camera* getActiveCamera() const;
        void setActiveCamera(Camera&);

        template<class T, class ... ARGS>
        T* addCamera(ARGS&&... args) {
            T* camera = NEW T(this, std::forward<ARGS>(args)...);
            m_Cameras.push_back(camera);
            if (!getActiveCamera()) {
                setActiveCamera(*camera);
            }
            return camera;
        }

        /*
        void addCamera(Camera&);
        [[nodiscard]] Camera* addCamera(float left, float right, float top, float bottom, float Near, float Far);
        [[nodiscard]] Camera* addCamera(float angle, float aspectRatio, float Near, float Far);
        */
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
        static std::vector<Entity>&              GetEntities(Scene& scene);
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

        static void                       RenderGeometryOpaque(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparent(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparentTrianglesSorted(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardOpaque(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardTransparent(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardTransparentTrianglesSorted(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardParticles(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderDecals(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderParticles(RenderModule&, Scene&, Viewport&, Camera&, Handle program);


        static void                       RenderGeometryOpaqueShadowMap(RenderModule&, Scene&, Viewport*, Camera*);
        static void                       RenderGeometryTransparentShadowMap(RenderModule&, Scene&, Viewport*, Camera*);
        static void                       RenderGeometryTransparentTrianglesSortedShadowMap(RenderModule&, Scene&, Viewport*, Camera*);
        static void                       RenderForwardOpaqueShadowMap(RenderModule&, Scene&, Viewport*, Camera*);
        static void                       RenderForwardTransparentShadowMap(RenderModule&, Scene&, Viewport*, Camera*);
        static void                       RenderForwardTransparentTrianglesSortedShadowMap(RenderModule&, Scene&, Viewport*, Camera*);



        static void                       AddModelInstanceToPipeline(Scene&, ModelInstance&, RenderStage);
        static void                       RemoveModelInstanceFromPipeline(Scene&, ModelInstance&, RenderStage);
        [[nodiscard]] static Engine::priv::ECS& GetECS(Scene&);
        static void                       CleanECS(Scene&, Entity);
        static void                       SkipRenderThisFrame(Scene&);
        [[nodiscard]] static bool         IsSkipRenderThisFrame(Scene&);
        [[nodiscard]] static bool         HasItemsToRender(Scene&);
    };
};

#endif