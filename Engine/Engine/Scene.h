#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "Engine_ResourceBasic.h"
#include "Engine_EventObject.h"
#include <unordered_map>
#include <glm/vec3.hpp>

class Entity;
class Camera;
class SunLight;
class SkyboxEmpty;
class LightProbe;
class Scene;
class MeshInstance;

namespace Engine {
    namespace epriv {
        class ComponentManager;
        class RenderPipeline;
        class InternalScenePublicInterface final {
            friend class ::Scene;
            friend class ::Engine::epriv::RenderPipeline;
            public:
                static std::vector<uint>& GetEntities(Scene&);
                static std::vector<SunLight*>& GetLights(Scene&);
                static void RenderGeometryOpaque(Scene&, Camera&);
                static void RenderGeometryTransparent(Scene&, Camera&);
                static void RenderForwardOpaque(Scene&, Camera&);
                static void RenderForwardTransparent(Scene&, Camera&);
                static void AddMeshInstanceToPipeline(Scene&, MeshInstance&, RenderStage::Stage);
                static void RemoveMeshInstanceFromPipeline(Scene&, MeshInstance&, RenderStage::Stage);
        };
    };
};
class Scene: public EngineResource, public EventObserver{
    friend class ::LightProbe;
    friend class ::Engine::epriv::RenderPipeline;
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Engine::epriv::InternalScenePublicInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Scene(std::string name);
        virtual ~Scene();

        Entity* getEntity(uint entityID);
        uint addEntity(Entity&);
        void removeEntity(Entity&,bool immediate = false);
        void removeEntity(uint id,bool immediate = false);
        bool hasEntity(Entity&);
        bool hasEntity(uint entityID);

        virtual void update(const float& dt);
        std::unordered_map<std::string,LightProbe*>& lightProbes();

        Camera* getActiveCamera();

        glm::vec3 getBackgroundColor();
        void setBackgroundColor(float,float,float);

        SkyboxEmpty* skybox() const;
        void setSkybox(SkyboxEmpty*);
        void centerSceneToObject(Entity&);
        void centerSceneToObject(uint entityID);
        void setActiveCamera(Camera&);
};
#endif
