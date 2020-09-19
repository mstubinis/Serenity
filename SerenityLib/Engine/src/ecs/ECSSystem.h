#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

namespace Engine::priv {
    template<typename ENTITY>
    class ECS;
}

#include <ecs/ECSComponentPool.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/scene/SceneOptions.h>

namespace Engine::priv {
    template<typename ...> class ECSSystem;
    template <class ENTITY>
    class ECSSystem<ENTITY> {
        friend class Engine::priv::ECS<ENTITY>;
        protected:
            std_func_update               SUF;
            std_func_component            CAE;
            std_func_component_removed    CRE;
            std_func_entity               EAS;
            std_func_scene                SEF;
            std_func_scene                SLF;
            //unsigned int                  m_SortValue = 0;
        public:
            ECSSystem()                                      = default;
            virtual ~ECSSystem()                             = default;

            ECSSystem(const ECSSystem&)                      = delete;
            ECSSystem& operator=(const ECSSystem&)           = delete;
            ECSSystem(ECSSystem&& other) noexcept            = delete;
            ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

            virtual void onUpdate(const float dt, Scene&) {}
            virtual void onComponentAddedToEntity(void*, ENTITY) {}
            virtual void onComponentRemovedFromEntity(ENTITY) {}
            virtual void onEntityAddedToScene(ENTITY, Scene&) {}
            virtual void onSceneLeft(Scene&) {}
            virtual void onSceneEntered(Scene&) {}
    };

    template <class ENTITY, class COMPONENT>
    class ECSSystem<ENTITY, COMPONENT> : public ECSSystem<ENTITY> {
        using super     = ECSSystem<ENTITY>;
        using CPoolType = ECSComponentPool<ENTITY, COMPONENT>;
        private:
            CPoolType& componentPool;
        public:
            ECSSystem(const SceneOptions& options, const ECSSystemCI& systemConstructor, ECS<ENTITY>& ecs) 
                : componentPool(ecs.template getPool<COMPONENT>())
            {
                super::SUF = std::move(systemConstructor.onUpdateFunction);
                super::CAE = std::move(systemConstructor.onComponentAddedToEntityFunction);
                super::CRE = std::move(systemConstructor.onComponentRemovedFromEntityFunction);
                super::EAS = std::move(systemConstructor.onEntityAddedToSceneFunction);
                super::SEF = std::move(systemConstructor.onSceneEnteredFunction);
                super::SLF = std::move(systemConstructor.onSceneLeftFunction);
            }
            ECSSystem()                                      = default;
            virtual ~ECSSystem()                             = default;

            ECSSystem(const ECSSystem&)                      = delete;
            ECSSystem& operator=(const ECSSystem&)           = delete;
            ECSSystem(ECSSystem&& other) noexcept            = delete;
            ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

            void onUpdate(const float dt, Scene& scene) noexcept override { 
                super::SUF(this, &componentPool, dt, scene); 
			}
            void onComponentAddedToEntity(void* component, ENTITY entity) noexcept override {
                super::CAE(this, component, entity);
			}
            void onComponentRemovedFromEntity(ENTITY entity) noexcept override {
                super::CRE(this, entity);
            }
            void onEntityAddedToScene(ENTITY entity, Scene& scene) noexcept override {
                super::EAS(this, &componentPool, entity, scene);
			}
            void onSceneEntered(Scene& scene) noexcept override {
                super::SEF(this, &componentPool, scene);
			}
            void onSceneLeft(Scene& scene) noexcept override {
                super::SLF(this, &componentPool, scene); 
			}
    };
};
#endif