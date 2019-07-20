#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/events/Engine_EventObject.h>
#include <vector>
#include <glm/vec4.hpp>

class  Camera;

class  SunLight;
class  DirectionalLight;
class  PointLight;
class  SpotLight;
class  RodLight;

class  Viewport;
class  SkyboxEmpty;
class  MeshInstance;
struct Entity;

namespace Engine {
namespace epriv {
    class  RenderPipeline;
    struct InternalScenePublicInterface;
    struct EntityPOD;
    template<typename T> class ECS;
};
};
class Scene: public EngineResource, public EventObserver{
    friend class  Engine::epriv::RenderPipeline;
    friend struct Engine::epriv::InternalScenePublicInterface;
    private:
        struct impl; std::unique_ptr<impl> m_i;
    public:
        Scene(const std::string& name);
        virtual ~Scene();

        const uint& id() const;

        //new ecs
        Entity createEntity();
        Entity getEntity(const Engine::epriv::EntityPOD&);
        void removeEntity(const uint entityID);
        void removeEntity(Entity& entity);

        virtual void update(const double& dt);

        Viewport* addViewport(const uint& x, const uint& y, const uint& width, const uint& height, const Camera& camera);

        Camera* getActiveCamera();
        Viewport& getMainViewport();

        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const float& r, const float& g, const float& b, const float& a);
        void setGlobalIllumination(const float global, const float diffuse, const float specular);
        virtual void onEvent(const Event& e);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Entity&);
        void setActiveCamera(Camera&);
};
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface final {
            friend class ::Scene;
            friend class Engine::epriv::RenderPipeline;
            static std::vector<EntityPOD>& GetEntities(Scene&);

            static std::vector<Viewport*>&         GetViewports(Scene&);
            static std::vector<Camera*>&           GetCameras(Scene&);
            static std::vector<SunLight*>&         GetSunLights(Scene&);
            static std::vector<DirectionalLight*>& GetDirectionalLights(Scene&);
            static std::vector<PointLight*>&       GetPointLights(Scene&);
            static std::vector<SpotLight*>&        GetSpotLights(Scene&);
            static std::vector<RodLight*>&         GetRodLights(Scene&);

            static void RenderGeometryOpaque(Scene&, Camera&);
            static void RenderGeometryTransparent(Scene&, Camera&);
            static void RenderForwardOpaque(Scene&, Camera&);
            static void RenderForwardTransparent(Scene&, Camera&);
            static void AddMeshInstanceToPipeline(Scene&, MeshInstance&, RenderStage::Stage);
            static void RemoveMeshInstanceFromPipeline(Scene&, MeshInstance&, RenderStage::Stage);
            static ECS<Entity>& GetECS(Scene&);
            static uint NumScenes;
        };
    };
};

#endif