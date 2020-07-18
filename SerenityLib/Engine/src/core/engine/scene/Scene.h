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
struct Entity;
struct SceneOptions;

class ParticleEmitter;
class ParticleEmissionProperties;
class Particle;

namespace Engine::priv {
    class  RenderGraph;
    class  ResourceManager;
    struct InternalScenePublicInterface;
    template<typename T> class ECS;
    class  GBuffer;
    class  Renderer;
    class  EngineCore;
};

#include <core/engine/renderer/RendererIncludes.h>
#include <core/engine/resources/Resource.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/events/Observer.h>

class Scene: public Resource, public Observer {
    friend class  Engine::priv::RenderGraph;
    friend class  Engine::priv::ResourceManager;
    friend struct Engine::priv::InternalScenePublicInterface;
    friend class  Engine::priv::EngineCore;
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
        mutable std::vector<ProjectionLight*>                         m_ProjectionLights;

        unsigned int                                                  m_ID                 = 0;
        glm::vec3                                                     m_GI                 = glm::vec3(1.0f);

        Entity*                                                       m_Sun                = nullptr;
        Skybox*                                                       m_Skybox             = nullptr;

        class impl; impl*                                             m_i                  = nullptr;
        std::function<void(Scene*, const float)>                      m_OnUpdateFunctor    = [](Scene*, const float) {};

        void preUpdate(const float dt);
        void postUpdate(const float dt);
    public:
        Scene(const std::string& name);
        Scene(const std::string& name, const SceneOptions& options);
        virtual ~Scene();


        void update(const float dt);
        virtual void render();
        virtual void onEvent(const Event& event);
        virtual void onResize(unsigned int width, unsigned int height);

        void setOnUpdateFunctor(std::function<void(Scene*, const float)> functor);

        unsigned int id() const;
        unsigned int numViewports() const;


        Entity createEntity();
        void removeEntity(Entity entity);

        
        Viewport& getMainViewport();
        Viewport& addViewport(float x, float y, float width, float height, const Camera& camera);

        ParticleEmitter* addParticleEmitter(ParticleEmissionProperties& properties, Scene& scene, float lifetime, Entity* parent = nullptr);


        Camera* getActiveCamera() const;
        void setActiveCamera(Camera& camera);

        void addCamera(Camera& camera);
        Camera* addCamera(float left, float right, float top, float bottom, float Near, float Far);
        Camera* addCamera(float angle, float aspectRatio, float Near, float Far);

        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(float r, float g, float b, float a);
        void setBackgroundColor(const glm::vec4& backgroundColor);

        const glm::vec3& getGlobalIllumination() const;
        void setGlobalIllumination(float global, float diffuse, float specular);
        void setGlobalIllumination(const glm::vec3& globalIllumination);

        void setGodRaysSun(Entity* sun);
        Entity* getGodRaysSun() const;

        Skybox* skybox() const;
        void setSkybox(Skybox* skybox);

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
        static std::vector<SunLight*>&           GetLights(const Scene& scene);
        static std::vector<SunLight*>&           GetSunLights(const Scene& scene);
        static std::vector<DirectionalLight*>&   GetDirectionalLights(const Scene& scene);
        static std::vector<PointLight*>&         GetPointLights(const Scene& scene);
        static std::vector<SpotLight*>&          GetSpotLights(const Scene& scene);
        static std::vector<RodLight*>&           GetRodLights(const Scene& scene);
        static std::vector<ProjectionLight*>&    GetProjectionLights(const Scene& scene);

        static void           UpdateMaterials(Scene& scene, const float dt);

        static void           RenderGeometryOpaque( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderGeometryTransparent( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderGeometryTransparentTrianglesSorted( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderForwardOpaque( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderForwardTransparent( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderForwardTransparentTrianglesSorted( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderForwardParticles( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderDecals( Renderer&, Scene& scene, Viewport&, Camera&, bool useDefaultShaders = true);
        static void           RenderParticles( Renderer&, Scene& scene, Viewport&, Camera&, ShaderProgram& program);

        static void           AddModelInstanceToPipeline(Scene& scene, ModelInstance&, RenderStage::Stage stage);
        static void           RemoveModelInstanceFromPipeline(Scene& scene, ModelInstance&, RenderStage::Stage stage);
        static ECS<Entity>&   GetECS(const Scene& scene);
        static void           CleanECS(Scene& scene, Entity entity);
    };
};

#endif