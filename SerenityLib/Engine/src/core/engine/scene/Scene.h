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
struct Entity;
struct SceneOptions;

class ParticleEmitter;
class ParticleEmissionProperties;
class Particle;

namespace Engine::priv {
    class  ECS;
    class  RenderGraph;
    class  ResourceManager;
    struct InternalScenePublicInterface;
    class  GBuffer;
    class  RenderModule;
    class  EngineCore;
};
#include <ecs/ECS.h>
#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/events/Observer.h>
#include <core/engine/resources/Handle.h>

class Scene: public Observer {
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::InternalScenePublicInterface;
    friend class  Engine::priv::EngineCore;
    using UpdateFP = void(*)(Scene*, const float);
    private:
        mutable std::vector<Viewport>                                 m_Viewports;
        mutable std::vector<Camera*>                                  m_Cameras;
        mutable std::vector<std::vector<Engine::priv::RenderGraph>>   m_RenderGraphs;

        mutable std::vector<SunLight*>                                m_SunLights;
        mutable std::vector<DirectionalLight*>                        m_DirectionalLights;
        mutable std::vector<PointLight*>                              m_PointLights;
        mutable std::vector<SpotLight*>                               m_SpotLights;
        mutable std::vector<RodLight*>                                m_RodLights;
        mutable std::vector<ProjectionLight*>                         m_ProjectionLights;

        UpdateFP                                                      m_OnUpdateFunctor     = [](Scene*, const float) {};
        std::string                                                   m_Name                = "";
        unsigned int                                                  m_ID                  = 0;
        glm::vec3                                                     m_GI                  = glm::vec3(1.0f);
        bool                                                          m_SkipRenderThisFrame = false;

        Entity                                                        m_Sun                 = Entity{};
        Skybox*                                                       m_Skybox              = nullptr;

        class impl; std::unique_ptr<impl>                             m_i                   = nullptr;

        void preUpdate(const float dt);
        void postUpdate(const float dt);
    public:
        Scene(const std::string& name);
        Scene(const std::string& name, const SceneOptions& options);
        virtual ~Scene();

        inline void setName(const std::string& name) noexcept { m_Name = name; }
        inline void setName(std::string&& name) noexcept { m_Name = std::move(name); }
        inline void setName(const char* name) noexcept { m_Name = name; }
        inline CONSTEXPR const std::string& name() const noexcept { return m_Name; }


        template<typename ... ARGS> inline Engine::view_ptr<SunLight> createSunLight(ARGS&& ... args) {
            return m_SunLights.emplace_back(NEW SunLight(this, std::forward<ARGS>(args)...));
        }
        template<typename ... ARGS> inline Engine::view_ptr<DirectionalLight> createDirectionalLight(ARGS&& ... args) {
            return m_DirectionalLights.emplace_back(NEW DirectionalLight(this, std::forward<ARGS>(args)...));
        }
        template<typename ... ARGS> inline Engine::view_ptr<PointLight> createPointLight(ARGS&& ... args) {
            return m_PointLights.emplace_back(NEW PointLight(this, std::forward<ARGS>(args)...));
        }
        template<typename ... ARGS> inline Engine::view_ptr<SpotLight> createSpotLight(ARGS&& ... args) {
            return m_SpotLights.emplace_back(NEW SpotLight(this, std::forward<ARGS>(args)...));
        }
        template<typename ... ARGS> inline Engine::view_ptr<RodLight> createRodLight(ARGS&& ... args) {
            return m_RodLights.emplace_back(NEW RodLight(this, std::forward<ARGS>(args)...));
        }
        template<typename ... ARGS> inline Engine::view_ptr<ProjectionLight> createProjectionLight(ARGS&& ... args) {
            return m_ProjectionLights.emplace_back(NEW ProjectionLight(this, std::forward<ARGS>(args)...));
        }

        void deleteSunLight(SunLight* light);
        void deleteDirectionalLight(DirectionalLight* light);
        void deletePointLight(PointLight* light);
        void deleteSpotLight(SpotLight* light);
        void deleteRodLight(RodLight* light);
        void deleteProjectionLight(ProjectionLight* light);


        void clearAllEntities() noexcept;
        void update(const float dt);
        virtual void render() {}
        virtual void onEvent(const Event& event);
        virtual void onResize(unsigned int width, unsigned int height) {}

        inline void setOnUpdateFunctor(const UpdateFP& functor) noexcept { m_OnUpdateFunctor = functor; }
        inline void setOnUpdateFunctor(UpdateFP&& functor) noexcept { m_OnUpdateFunctor = std::move(functor); }

        inline CONSTEXPR unsigned int id() const noexcept { return m_ID; }
        inline unsigned int numViewports() const noexcept { return (unsigned int)m_Viewports.size(); }


        Entity createEntity();
        void removeEntity(Entity entity);

        size_t getNumLights() const noexcept;

        Viewport& getMainViewport();
        Viewport& addViewport(float x, float y, float width, float height, Camera& camera);

        ParticleEmitter* addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity* parent = nullptr);


        Camera* getActiveCamera() const;
        void setActiveCamera(Camera& camera);

        void addCamera(Camera& camera);
        Camera* addCamera(float left, float right, float top, float bottom, float Near, float Far);
        Camera* addCamera(float angle, float aspectRatio, float Near, float Far);

        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(float r, float g, float b, float a);
        void setBackgroundColor(const glm::vec4& backgroundColor);

        inline CONSTEXPR const glm::vec3& getGlobalIllumination() const noexcept { return m_GI; }
        void setGlobalIllumination(float global, float diffuse, float specular);
        void setGlobalIllumination(const glm::vec3& globalIllumination);

        inline void setGodRaysSun(Entity sun) noexcept { m_Sun = sun; }
        inline CONSTEXPR Entity getGodRaysSun() const noexcept { return m_Sun; }
        inline CONSTEXPR Skybox* skybox() const noexcept { return m_Skybox; }
        inline void setSkybox(Skybox* s) noexcept { m_Skybox = s; }

        void centerSceneToObject(Entity centerEntity);
};
namespace Engine::priv {
    struct InternalScenePublicInterface final {
        friend class Scene;
        friend class Engine::priv::RenderGraph;

        static std::vector<Particle>&            GetParticles(const Scene& scene);
        static std::vector<Entity>&              GetEntities(const Scene& scene);
        static std::vector<Viewport>&            GetViewports(const Scene& scene);
        static std::vector<Camera*>&             GetCameras(const Scene& scene);
        static std::vector<SunLight*>&           GetSunLights(const Scene& scene);
        static std::vector<DirectionalLight*>&   GetDirectionalLights(const Scene& scene);
        static std::vector<PointLight*>&         GetPointLights(const Scene& scene);
        static std::vector<SpotLight*>&          GetSpotLights(const Scene& scene);
        static std::vector<RodLight*>&           GetRodLights(const Scene& scene);
        static std::vector<ProjectionLight*>&    GetProjectionLights(const Scene& scene);

        static void                       UpdateMaterials(Scene& scene, const float dt);

        static void                       RenderGeometryOpaque(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparent(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderGeometryTransparentTrianglesSorted(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardOpaque(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardTransparent(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardTransparentTrianglesSorted(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderForwardParticles(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderDecals(RenderModule&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void                       RenderParticles(RenderModule&, Scene& scene, Viewport&, Camera&, Handle program);

        static void                       AddModelInstanceToPipeline(Scene& scene, ModelInstance&, RenderStage stage, ComponentModel&);
        static void                       RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance&, RenderStage stage);
        static Engine::priv::ECS& GetECS(Scene& scene);
        static void                       CleanECS(Scene& scene, Entity entity);
        static void                       SkipRenderThisFrame(Scene& scene, bool isSkip);
        static bool                       IsSkipRenderThisFrame(Scene& scene);
    };
};

#endif