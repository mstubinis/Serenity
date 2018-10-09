#pragma once
#ifndef ENGINE_SCENE_H_INCLUDE_GUARD
#define ENGINE_SCENE_H_INCLUDE_GUARD

#include "core/engine/Engine_ResourceBasic.h"
#include "core/engine/Engine_EventObject.h"
#include <unordered_map>
#include <glm/vec3.hpp>


class OLD_Entity;
class Camera;
class SunLight;
class SkyboxEmpty;
class MeshInstance;
struct Entity;

namespace Engine {
    namespace epriv {
        class OLD_ComponentManager;
        class RenderPipeline;
        struct InternalScenePublicInterface;
        class ECS;
    };
};
class Scene: public EngineResource, public EventObserver{
    friend class  Engine::epriv::RenderPipeline;
    friend class  Engine::epriv::OLD_ComponentManager;
    friend struct Engine::epriv::InternalScenePublicInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Scene(std::string name);
        virtual ~Scene();

        uint id();
        OLD_Entity* OLD_getEntity(uint entityID);
        uint        OLD_addEntity(OLD_Entity&);
        void        OLD_removeEntity(OLD_Entity&);
        void        OLD_removeEntity(uint id);
        bool        OLD_hasEntity(OLD_Entity&);
        bool        OLD_hasEntity(uint entityID);

        virtual void update(const float& dt);

        Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(OLD_Entity&);
        void centerSceneToObject(uint entityID);
        void setActiveCamera(Camera&);
};
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface final {
            friend class ::Scene;
            friend class Engine::epriv::RenderPipeline;
            static std::vector<uint>& OLD_GetEntities(Scene&);
            static std::vector<SunLight*>& GetLights(Scene&);
            static void RenderGeometryOpaque(Scene&, Camera&);
            static void RenderGeometryTransparent(Scene&, Camera&);
            static void RenderForwardOpaque(Scene&, Camera&);
            static void RenderForwardTransparent(Scene&, Camera&);
            static void AddMeshInstanceToPipeline(Scene&, MeshInstance&, RenderStage::Stage);
            static void RemoveMeshInstanceFromPipeline(Scene&, MeshInstance&, RenderStage::Stage);
            static ECS& GetECS(Scene&);

            static uint NumScenes;
        };
    };
};


#endif
