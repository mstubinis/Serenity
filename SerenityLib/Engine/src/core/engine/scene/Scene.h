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
};

#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/renderer/particles/ParticleSystem.h>
#include <functional>

class Scene: public EngineResource, public EventObserver{
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::InternalScenePublicInterface;
    public:
        virtual void update(const float& dt);
        virtual void render();
        virtual void onEvent(const Event& _event);
        virtual void onResize(const unsigned int& width, const unsigned int& height);

        template<typename T> void setOnUpdateFunctor(const T& functor) {
            m_OnUpdateFunctor = std::bind<void>(functor, this, std::placeholders::_1);
        }
    private:
        std::vector<Viewport*>                                m_Viewports;
        std::vector<Camera*>                                  m_Cameras;
        std::vector<std::vector<Engine::priv::RenderGraph*>>  m_RenderGraphs;

        std::vector<SunLight*>                                m_Lights;
        std::vector<SunLight*>                                m_SunLights;
        std::vector<DirectionalLight*>                        m_DirectionalLights;
        std::vector<PointLight*>                              m_PointLights;
        std::vector<SpotLight*>                               m_SpotLights;
        std::vector<RodLight*>                                m_RodLights;

        std::vector<unsigned int>              m_Entities;
        unsigned int                           m_ID;
        glm::vec3                              m_GI;

        Entity* m_Sun;
        Skybox* m_Skybox;

        class impl; impl*                      m_i;
        std::function<void(const float&)>     m_OnUpdateFunctor;
    public:
        Scene(const std::string& name);
        Scene(const std::string& name, const SceneOptions& options);
        virtual ~Scene();

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
        Entity* getGodRaysSun();

        Skybox* skybox() const;
        void setSkybox(Skybox*);

        void centerSceneToObject(const Entity& centerEntity);
};
namespace Engine {
    namespace priv {
        struct InternalScenePublicInterface final {
            friend class Scene;
            friend class Engine::priv::RenderGraph;

            static std::vector<Particle>&            GetParticles(Scene&);
            static std::vector<EntityPOD>&           GetEntities(Scene&);
            static std::vector<Viewport*>&           GetViewports(Scene&);
            static std::vector<Camera*>&             GetCameras(Scene&);
            static std::vector<SunLight*>&           GetLights(Scene&);
            static std::vector<SunLight*>&           GetSunLights(Scene&);
            static std::vector<DirectionalLight*>&   GetDirectionalLights(Scene&);
            static std::vector<PointLight*>&         GetPointLights(Scene&);
            static std::vector<SpotLight*>&          GetSpotLights(Scene&);
            static std::vector<RodLight*>&           GetRodLights(Scene&);

            static void           UpdateMaterials(Scene&, const float& dt);
            static void           UpdateParticleSystem(Scene&, const float& dt);
            static void           RenderGeometryOpaque(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderGeometryTransparent(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderGeometryTransparentTrianglesSorted(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardOpaque(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardTransparent(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardTransparentTrianglesSorted(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderForwardParticles(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderDecals(Scene&, Viewport&, Camera&, const bool useDefaultShaders = true);
            static void           RenderParticles(Scene&, Viewport&, Camera&, ShaderProgram& program, GBuffer&);

            static void           AddModelInstanceToPipeline(Scene&, ModelInstance&, const RenderStage::Stage& stage);
            static void           RemoveModelInstanceFromPipeline(Scene&, ModelInstance&, const RenderStage::Stage& stage);
            static ECS<Entity>&   GetECS(Scene&);
            static void           CleanECS(Scene&, const unsigned int entityData);
        };
    };
};

#endif