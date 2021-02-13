#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

struct SceneOptions;
namespace Engine::priv {
    template<class COMPONENT> class ECSComponentPool;
};

#include <serenity/ecs/ECSSystemConstructorInfo.h>
#include <serenity/containers/SparseSet.h>

namespace Engine::priv {
    class ECSSystemBase {
        friend class Engine::priv::ECS;
        protected:
            std_func_update               SUF;
            std_func_component            CAE;
            std_func_component_removed    CRE;
            std_func_entity               EAS;
            std_func_scene                SEF;
            std_func_scene                SLF;
        public:
            ECSSystemBase()                                      = default;
            virtual ~ECSSystemBase()                             = default;

            ECSSystemBase(const ECSSystemBase&)                      = delete;
            ECSSystemBase& operator=(const ECSSystemBase&)           = delete;
            ECSSystemBase(ECSSystemBase&& other) noexcept            = delete;
            ECSSystemBase& operator=(ECSSystemBase&& other) noexcept = delete;

            inline virtual void onUpdate(const float dt, Scene&) {}
            inline virtual void onComponentAddedToEntity(void*, Entity) {}
            inline virtual void onComponentRemovedFromEntity(Entity) {}
            inline virtual void onEntityAddedToScene(Entity, Scene&) {}
            inline virtual void onSceneLeft(Scene&) {}
            inline virtual void onSceneEntered(Scene&) {}
    };

    template <class COMPONENT>
    class ECSSystem : public ECSSystemBase {
        private:
            ECSComponentPool<COMPONENT>& componentPool;
        public:
            ECSSystem(const SceneOptions& options, const ECSSystemCI& systemConstructor, Engine::priv::sparse_set_base& inComponentPool)
                : componentPool{ static_cast<ECSComponentPool<COMPONENT>&>(inComponentPool) }
            {
                SUF = std::move(systemConstructor.onUpdateFunction);
                CAE = std::move(systemConstructor.onComponentAddedToEntityFunction);
                CRE = std::move(systemConstructor.onComponentRemovedFromEntityFunction);
                EAS = std::move(systemConstructor.onEntityAddedToSceneFunction);
                SEF = std::move(systemConstructor.onSceneEnteredFunction);
                SLF = std::move(systemConstructor.onSceneLeftFunction);
            }
            ECSSystem()                                      = default;
            virtual ~ECSSystem()                             = default;

            ECSSystem(const ECSSystem&)                      = delete;
            ECSSystem& operator=(const ECSSystem&)           = delete;
            ECSSystem(ECSSystem&& other) noexcept            = delete;
            ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

            inline void onUpdate(const float dt, Scene& scene) noexcept override {
                ECSSystemBase::SUF(this, &componentPool, dt, scene);
			}
            inline void onComponentAddedToEntity(void* component, Entity entity) noexcept override {
                ECSSystemBase::CAE(this, component, entity);
			}
            inline void onComponentRemovedFromEntity(Entity entity) noexcept override {
                ECSSystemBase::CRE(this, entity);
            }
            inline void onEntityAddedToScene(Entity entity, Scene& scene) noexcept override {
                ECSSystemBase::EAS(this, &componentPool, entity, scene);
			}
            inline void onSceneEntered(Scene& scene) noexcept override {
                ECSSystemBase::SEF(this, &componentPool, scene);
			}
            inline void onSceneLeft(Scene& scene) noexcept override {
                ECSSystemBase::SLF(this, &componentPool, scene);
			}
    };
};
#endif