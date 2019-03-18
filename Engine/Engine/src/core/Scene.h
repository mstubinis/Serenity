#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "core/engine/Engine_ResourceBasic.h"
#include "core/engine/events/Engine_EventObject.h"
#include <unordered_map>
#include <glm/vec3.hpp>

class Camera;
class SunLight;
class SkyboxEmpty;
class MeshInstance;
struct Entity;

namespace Engine {
    namespace epriv {
        class RenderPipeline;
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
        Scene(std::string name);
        virtual ~Scene();

        uint id();

        //new ecs
        Entity createEntity();
        Entity getEntity(Engine::epriv::EntityPOD&);
        void removeEntity(uint entityID);
        void removeEntity(Entity& entity);

        virtual void update(const float& dt);

        Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

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
            static std::vector<SunLight*>& GetLights(Scene&);
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