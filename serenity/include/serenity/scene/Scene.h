#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

class  Scene;
struct SceneImpl;
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
    class  ParticleSystemNew;
};
#include <serenity/ecs/ECS.h>
#include <serenity/renderer/RendererIncludes.h>
#include <serenity/scene/Viewport.h>
#include <serenity/events/Observer.h>
#include <serenity/resources/Handle.h>
#include <serenity/scene/LightsModule.h>
#include <serenity/renderer/RenderGraph.h>

//old
#include <serenity/renderer/particles/ParticleSystem.h>
//new
//#include <serenity/renderer/particles/ParticleSystemNew.h>
#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>
#include <serenity/renderer/particles/ParticleEmitterHandle.h>
#include <serenity/renderer/particles/ParticleEmitterContainer.h>

#include <serenity/scene/Camera.h>

class Scene: public Observer {
    friend struct ::SceneImpl;
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::PublicScene;
    friend class  Engine::priv::EngineCore;
    friend class  ::SystemSceneUpdate;
    friend class  ::SystemSceneChanging;
    friend class  ::SystemAddRigidBodies;
    friend class  ::SystemRemoveRigidBodies;
    using UpdateFP = void(*)(Scene*, const float);
    private:
        Engine::priv::ECS                           m_ECS;

        //old
        mutable Engine::priv::ParticleSystem        m_ParticleSystem;
        //new
        Engine::priv::ParticleSystemNew*             m_ParticleSystemNew = nullptr;





        mutable std::vector<Viewport>               m_Viewports;
        mutable std::vector<Camera*>                m_Cameras;
        mutable Engine::priv::RenderGraphContainer  m_RenderGraphs;
        mutable Engine::priv::LightsModule          m_LightsModule;
        UpdateFP                                    m_OnUpdateFunctor     = [](Scene*, const float) {};
        glm::vec3                                   m_GI                  = glm::vec3{ 1.0f };
        glm::vec3                                   m_AmbientColor        = glm::vec3{ 0.05f, 0.05f, 0.05f };
        std::string                                 m_Name;
        Skybox*                                     m_Skybox              = nullptr;
        uint32_t                                    m_ID                  = std::numeric_limits<uint32_t>().max();
        Entity                                      m_Sun;
        bool                                        m_SkipRenderThisFrame = false;
        bool                                        m_WasJustSwappedTo    = false;

        void internal_register_lights();
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
        inline void registerSystem(ARG_TUPLE&& argTuple) {
            m_ECS.registerSystem<SYSTEM, ARG_TUPLE, COMPONENTS...>(std::forward<decltype(argTuple)>(argTuple));
        }
        template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
        inline void registerSystemOrdered(int32_t order, ARG_TUPLE&& argTuple) {
            m_ECS.registerSystemOrdered<SYSTEM, ARG_TUPLE, COMPONENTS...>(order, std::forward<decltype(argTuple)>(argTuple));
        }
        template<class SYSTEM, class ... COMPONENTS>
        inline void registerSystem() {
            m_ECS.registerSystem<SYSTEM, COMPONENTS...>();
        }
        template<class SYSTEM, class ... COMPONENTS>
        inline void registerSystemOrdered(int32_t order) {
            m_ECS.registerSystemOrdered<SYSTEM, COMPONENTS...>(order);
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
        [[nodiscard]] inline uint32_t getNumViewports() const noexcept { return static_cast<uint32_t>(m_Viewports.size()); }


        [[nodiscard]] Entity createEntity();
        [[nodiscard]] std::vector<Entity> createEntity(uint32_t amount);
        [[nodiscard]] std::vector<Entity> createEntities(uint32_t amount);
        void removeEntity(Entity);

        [[nodiscard]] size_t getNumLights() const noexcept;

        [[nodiscard]] Viewport& getMainViewport();
        Viewport& addViewport(float x, float y, float width, float height, Camera&);

        [[nodiscard]] Engine::view_ptr<ParticleEmitter> addParticleEmitter(ParticleEmissionPropertiesHandle, float lifetime, Entity parent = {});
        [[nodiscard]] ParticleEmitterHandle addParticleEmitterNew(ParticleEmissionPropertiesHandle, float lifetime);


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
        friend class  ::Scene;
        friend struct ::SceneImpl;
        friend class  Engine::priv::RenderGraph;


        //old
        static std::vector<Particle>&            GetParticles(const Scene&);
        //new
        static Engine::priv::ParticleEmitterContainer<ParticleEmitter, ParticleEmitterHandle>& GetParticleEmitters(Scene&);

        static std::vector<Entity>&              GetEntities(Scene&);
        static std::vector<Viewport>&            GetViewports(const Scene&);
        static std::vector<Camera*>&             GetCameras(const Scene&);

        template<class LIGHT>
        [[nodiscard]] static inline Engine::priv::LightContainer<LIGHT>& GetLights(const Scene& scene) noexcept {
            return scene.m_LightsModule.getLights<LIGHT>(); 
        }
        template<class LIGHT>
        static inline bool SetLightShadowCaster(const Scene& scene, LIGHT& light, bool isShadowCaster) noexcept {
            return scene.m_LightsModule.setShadowCaster(&light, isShadowCaster);
        }

        static void                       UpdateParticles(Scene&, const float dt);
        static void                       UpdateMaterials(Scene&, const float dt);

        static void                       RenderGeometryOpaque(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparent(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparentTrianglesSorted(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardOpaque(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardTransparent(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardTransparentTrianglesSorted(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderForwardParticles(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderDecals(RenderModule&, Scene&, Viewport*, Camera*, bool useDefaultShaders = true);
        static void                       RenderParticles(RenderModule&, Scene&, Viewport&, Camera&);


        static void                       RenderGeometryOpaqueShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);
        static void                       RenderGeometryTransparentShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);
        static void                       RenderGeometryTransparentTrianglesSortedShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);
        static void                       RenderForwardOpaqueShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);
        static void                       RenderForwardTransparentShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);
        static void                       RenderForwardTransparentTrianglesSortedShadowMap(RenderModule&, Scene&, Viewport*, const glm::mat4& viewProjMatrix);



        static void                       AddModelInstanceToPipeline(Scene&, ModelInstance&, RenderStage);
        static void                       RemoveModelInstanceFromPipeline(Scene&, ModelInstance&, RenderStage);
        [[nodiscard]] static Engine::priv::ECS& GetECS(Scene&);
        [[nodiscard]] static Engine::priv::ECS& GetECS(Entity);
        static void                       CleanECS(Scene&, Entity);
        static void                       SkipRenderThisFrame(Scene&);
        [[nodiscard]] static bool         IsSkipRenderThisFrame(Scene&);
        [[nodiscard]] static bool         HasItemsToRender(Scene&);
    };
};


namespace Engine::priv {
    class SceneLUABinder {
        private:
            Engine::view_ptr<Scene> m_Scene = nullptr;
        public:
            SceneLUABinder() = default;
            SceneLUABinder(Scene&);

            [[nodiscard]] Scene& getScene() noexcept;

            void setName(const std::string& name);
            Entity createEntity();

            Engine::priv::CameraLUABinder getActiveCamera() const;
            void setActiveCamera(Engine::priv::CameraLUABinder);

            uint32_t id() const;
    };
}

/*
        //scene stuff
        .beginClass<Engine::priv::SceneLUABinder>("Scene")
            //.addProperty("name", &Scene::name, &Scene::setName)
            .addFunction("createEntities", static_cast<std::vector<Entity>(Scene::*)(uint32_t)>(&Scene::createEntity))
            .addFunction("centerSceneToObject", &Scene::centerSceneToObject)
            .addFunction("getBackgroundColor", &Scene::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Scene::*)(float, float, float, float)>(&Scene::setBackgroundColor))
            .addFunction("setGlobalIllumination", static_cast<void(Scene::*)(float, float, float)>(&Scene::setGlobalIllumination))
            .addFunction("getGlobalIllumination", &Scene::getGlobalIllumination)
        .endClass()
        .addFunction("getCurrentScene", &Engine::Resources::getCurrentScene)
        .addFunction("setCurrentScene", static_cast<bool(*)(Scene*)>(&Engine::Resources::setCurrentScene))
        .addFunction("setCurrentSceneByName", static_cast<bool(*)(std::string_view)>(&Engine::Resources::setCurrentScene))
*/

#endif