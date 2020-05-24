#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

namespace Engine::priv {
    template<typename TEntity>
    class ECS;
}

#include <ecs/ECSComponentPool.h>
#include <ecs/ECSSystemConstructorInfo.h>

namespace Engine::priv {
    template<typename ...> class ECSSystem;
    template <class TEntity> 
    class ECSSystem<TEntity> {
        friend class Engine::priv::ECS<TEntity>;
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
            virtual void onComponentAddedToEntity(void*, TEntity&) {}
            virtual void onComponentRemovedFromEntity(TEntity&) {}
            virtual void onEntityAddedToScene(TEntity&, Scene&) {}
            virtual void onSceneLeft(Scene&) {}
            virtual void onSceneEntered(Scene&) {}
    };

    template <class TEntity, class TComponent> 
    class ECSSystem<TEntity, TComponent> : public ECSSystem<TEntity> {
        using super     = ECSSystem<TEntity>;
        using CPoolType = ECSComponentPool<TEntity, TComponent>;
        private:
            CPoolType& componentPool;

            void Bind_SUF(const FunctorUpdate& f) { 
                super::SUF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			}
            void Bind_CAE(const FunctorComponent& f) {
                super::CAE = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			}
            void Bind_CRE(const FunctorComponentRemoved& f) {
                super::CRE = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2);
            }
            void Bind_EAS(const FunctorEntity& f) {
                super::EAS = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
			}
            void Bind_SEF(const FunctorScene& f) {
                super::SEF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			}
            void Bind_SLF(const FunctorScene& f) {
                super::SLF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			}
        public:
            ECSSystem(const ECSSystemCI& systemCI, ECS<TEntity>& ecs) : componentPool(ecs.template getPool<TComponent>()){
                Bind_SUF(systemCI.onUpdateFunction);
                Bind_CAE(systemCI.onComponentAddedToEntityFunction);
                Bind_CRE(systemCI.onComponentRemovedFromEntityFunction);
                Bind_EAS(systemCI.onEntityAddedToSceneFunction);
                Bind_SEF(systemCI.onSceneEnteredFunction);
                Bind_SLF(systemCI.onSceneLeftFunction);
            }
            ECSSystem()                                      = default;
            virtual ~ECSSystem()                             = default;

            ECSSystem(const ECSSystem&)                      = delete;
            ECSSystem& operator=(const ECSSystem&)           = delete;
            ECSSystem(ECSSystem&& other) noexcept            = delete;
            ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

            void onUpdate(const float dt, Scene& scene) { 
                super::SUF(this, &componentPool, dt, scene); 
			}
            void onComponentAddedToEntity(void* component, TEntity& entity) { 
                super::CAE(this, component, entity);
			}
            void onComponentRemovedFromEntity(TEntity& entity) {
                super::CRE(this, entity);
            }
            void onEntityAddedToScene(TEntity& entity, Scene& scene) { 
                super::EAS(this, &componentPool, entity, scene);
			}
            void onSceneEntered(Scene& scene) { 
                super::SEF(this, &componentPool, scene);
			}
            void onSceneLeft(Scene& scene) { 
                super::SLF(this, &componentPool, scene); 
			}
    };
};
#endif