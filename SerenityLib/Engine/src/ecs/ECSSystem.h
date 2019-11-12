#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <ecs/ECS.h>
#include <ecs/ECSSystemConstructorInfo.h>

namespace Engine {
    namespace epriv {
        template<typename ...> class ECSSystem;
        template <class TEntity> class ECSSystem<TEntity> {
            protected:
                std_func_update         _SUF;
                std_func_component      _CAE;
                std_func_entity         _EAS;
                std_func_scene          _SEF;
                std_func_scene          _SLF;
            public:
                ECSSystem()                                      = default;
                virtual ~ECSSystem()                             = default;
                ECSSystem(const ECSSystem&)                      = delete;
                ECSSystem& operator=(const ECSSystem&)           = delete;
                ECSSystem(ECSSystem&& other) noexcept            = delete;
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

                virtual void onUpdate(const double& dt, Scene&) {}
                virtual void onComponentAddedToEntity(void*, TEntity&) {}
                virtual void onEntityAddedToScene(TEntity&, Scene&) {}
                virtual void onSceneLeft(Scene&) {}
                virtual void onSceneEntered(Scene&) {}
        };

        template <class TEntity, class TComponent> class ECSSystem<TEntity,TComponent> final : public ECSSystem<TEntity> {
            using super     = ECSSystem<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;

                void Bind_SUF(const FunctorUpdate& f) { 
                    super::_SUF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			    }
                void Bind_CAE(const FunctorComponent& f) {
                    super::_CAE = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2);
			    }
                void Bind_EAS(const FunctorEntity& f) {
                    super::_EAS = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			    }
                void Bind_SEF(const FunctorScene& f) {
                    super::_SEF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2);
			    }
                void Bind_SLF(const FunctorScene& f) {
                    super::_SLF = std::bind(f.functor, std::placeholders::_1, std::placeholders::_2);
			    }
            public:
                ECSSystem(const ECSSystemCI& systemCI, ECS<TEntity>& ecs):componentPool(ecs.template getPool<TComponent>()){
                    Bind_SUF(systemCI.onUpdateFunction);
                    Bind_CAE(systemCI.onComponentAddedToEntityFunction);
                    Bind_EAS(systemCI.onEntityAddedToSceneFunction);
                    Bind_SEF(systemCI.onSceneEnteredFunction);
                    Bind_SLF(systemCI.onSceneLeftFunction);
                }
                ECSSystem()                                      = default;
                ~ECSSystem()                                     = default;
                ECSSystem(const ECSSystem&)                      = delete;
                ECSSystem& operator=(const ECSSystem&)           = delete;
                ECSSystem(ECSSystem&& other) noexcept            = delete;
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

                void onUpdate(const double& dt, Scene& scene) { 
                    super::_SUF(&componentPool, dt, scene); 
			    }
                void onComponentAddedToEntity(void* component, TEntity& entity) { 
                    super::_CAE(component, entity); 
			    }
                void onEntityAddedToScene(TEntity& entity, Scene& scene) { 
                    super::_EAS(&componentPool, entity, scene); 
			    }
                void onSceneEntered(Scene& scene) { 
                    super::_SEF(&componentPool, scene); 
			    }
                void onSceneLeft(Scene& scene) { 
                    super::_SLF(&componentPool, scene); 
			    }
        };
    };
};
#endif