#pragma once
#ifndef ENGINE_SCENE_H_INCLUDE_GUARD
#define ENGINE_SCENE_H_INCLUDE_GUARD

class  Scene;
class  Camera;
class  SunLight;
class  DirectionalLight;
class  PointLight;
class  SpotLight;
class  RodLight;
class  ShaderProgram;
class  Viewport;
class  Skybox;
class  ModelInstance;
struct Entity;
struct SceneOptions;

namespace Engine::priv {
    class  RenderGraph;
    class  ResourceManager;
    class  EntityPOD;
    struct InternalScenePublicInterface;
    template<typename T> class ECS;
    class  GBuffer;
    class  Renderer;
};

#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/scene/Viewport.h>
#include <functional>

class Scene: public EngineResource, public EventObserver{
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::InternalScenePublicInterface;

    private:
        mutable std::vector<Viewport>                                 m_Viewports;
        mutable std::vector<Camera*>                                  m_Cameras;
        mutable std::vector<std::vector<Engine::priv::RenderGraph>>   m_RenderGraphs;

        mutable std::vector<SunLight*>                                m_Lights;
        mutable std::vector<SunLight*>                                m_SunLights;
        mutable std::vector<DirectionalLight*>                        m_DirectionalLights;
        mutable std::vector<PointLight*>                              m_PointLights;
        mutable std::vector<SpotLight*>                               m_SpotLights;
        mutable std::vector<RodLight*>                                m_RodLights;

        std::vector<unsigned int>                             m_Entities;
        unsigned int                                          m_ID;
        glm::vec3                                             m_GI;

        Entity* m_Sun;
        Skybox* m_Skybox;

        class impl; impl*                                     m_i;
        std::function<void(Scene*, const float&)>             m_OnUpdateFunctor;
    public:
        Scene(const std::string& name);
        Scene(const std::string& name, const SceneOptions& options);
        virtual ~Scene();


        virtual void update(const float& dt);
        virtual void render();
        virtual void onEvent(const Event& event);
        virtual void onResize(const unsigned int& width, const unsigned int& height);

        template<typename T> void setOnUpdateFunctor(const T& functor) {
            m_OnUpdateFunctor = std::bind<void>(functor, std::placeholders::_1, std::placeholders::_2);
        }

        const unsigned int id() const;
        const unsigned int numViewports() const;

        //ecs
        Entity createEntity();
        Entity getEntity(const Engine::priv::EntityPOD&);
        void removeEntity(const unsigned int& entityID);
        void removeEntity(Entity& entity);

        
        Viewport& getMainViewport();
        Viewport& addViewport(const float x, const float y, const float width, const float height, const Camera& camera);
        ParticleEmitter* addParticleEmitter(ParticleEmitter& emitter);

        Camera* getActiveCamera() const;
        void setActiveCamera(Camera&);


        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const float& r, const float& g, const float& b, const float& a);
        void setBackgroundColor(const glm::vec4& backgroundColor);

        const glm::vec3& getGlobalIllumination() const;
        void setGlobalIllumination(const float global, const float diffuse, const float specular);
        void setGlobalIllumination(const glm::vec3& globalIllumination);

        void setGodRaysSun(Entity*);
        Entity* getGodRaysSun() const;

        Skybox* skybox() const;
        void setSkybox(Skybox*);

        void centerSceneToObject(const Entity& centerEntity);
};
namespace Engine {
    namespace priv {
        struct InternalScenePublicInterface final {
            friend class Scene;
            friend class Engine::priv::RenderGraph;

            static std::vector<Particle>&            GetParticles(const Scene& scene);
            static std::vector<EntityPOD>&           GetEntities(const Scene& scene);
            static std::vector<Viewport>&            GetViewports(const Scene& scene);
            static std::vector<Camera*>&             GetCameras(const Scene& scene);
            static std::vector<SunLight*>&           GetLights(const Scene& scene);
            static std::vector<SunLight*>&           GetSunLights(const Scene& scene);
            static std::vector<DirectionalLight*>&   GetDirectionalLights(const Scene& scene);
            static std::vector<PointLight*>&         GetPointLights(const Scene& scene);
            static std::vector<SpotLight*>&          GetSpotLights(const Scene& scene);
            static std::vector<RodLight*>&           GetRodLights(const Scene& scene);

            static void           UpdateMaterials(Scene& scene, const float& dt);
            static void           UpdateParticleSystem(Scene& scene, const float& dt);

            static void           RenderGeometryOpaque( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderGeometryTransparent( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderGeometryTransparentTrianglesSorted( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardOpaque( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardTransparent( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardTransparentTrianglesSorted( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardParticles( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderDecals( Renderer&, const Scene& scene, const Viewport&, const Camera&, const bool useDefaultShaders = true);
            static void           RenderParticles( Renderer&, const Scene& scene, const Viewport&, const Camera&, ShaderProgram& program, const GBuffer&);

            static void           AddModelInstanceToPipeline(Scene& scene, ModelInstance&, const RenderStage::Stage& stage);
            static void           RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance&, const RenderStage::Stage& stage);
            static ECS<Entity>&   GetECS(const Scene& scene);
            static void           CleanECS(Scene& scene, const unsigned int entityData);
        };
    };
};

#endif