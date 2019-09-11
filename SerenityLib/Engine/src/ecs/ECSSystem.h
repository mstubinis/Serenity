#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <ecs/ECS.h>

#include <functional>

struct Entity;
class Scene;
namespace Engine {
namespace epriv {
    template<typename ...> class ECSSystem;

    struct FunctorUpdateEmpty final { void operator()(void* cPool, const double& dt, Scene&) const { } };
    struct FunctorComponentEmpty final { void operator()(void* compt, Entity&) const { } };
    struct FunctorEntityEmpty final { void operator()(void* cPool, Entity&, Scene&) const { } };
    struct FunctorSceneEmpty final { void operator()(void* cPool, Scene&) const { } };

    typedef std::function<void(void*, const double&, Scene&)>  std_func_update;
    typedef std::function<void(void*, Entity&, Scene&)>        std_func_entity;
    typedef std::function<void(void*, Entity&)>                std_func_component;
    typedef std::function<void(void*, Scene&)>                 std_func_scene;

    template<class T> struct FunctorHolder {
        T functor;
        FunctorHolder() {}
        FunctorHolder(const T& f) { functor = f; }

        FunctorHolder(const FunctorHolder&)                      = default;
        FunctorHolder& operator=(const FunctorHolder&)           = default;
        FunctorHolder(FunctorHolder&& other) noexcept            = default;
        FunctorHolder& operator=(FunctorHolder&& other) noexcept = default;

        virtual ~FunctorHolder() = default;
    };
    struct FunctorUpdate final : public FunctorHolder<std_func_update>{
        FunctorUpdate() { FunctorHolder<std_func_update>::functor = FunctorUpdateEmpty(); }
        FunctorUpdate(const std_func_update& f) { FunctorHolder<std_func_update>::functor = f; }
    };
    struct FunctorComponent : public FunctorHolder<std_func_component> {
        FunctorComponent() { FunctorHolder<std_func_component>::functor = FunctorComponentEmpty(); }
        FunctorComponent(const std_func_component& f) { FunctorHolder<std_func_component>::functor = f; }
    };
    struct FunctorEntity final : public FunctorHolder<std_func_entity> {
        FunctorEntity() { FunctorHolder<std_func_entity>::functor = FunctorEntityEmpty(); }
        FunctorEntity(const std_func_entity& f) { FunctorHolder<std_func_entity>::functor = f; }
    };
    struct FunctorScene final : public FunctorHolder<std_func_scene> {
        FunctorScene() { FunctorHolder<std_func_scene>::functor = FunctorSceneEmpty(); }
        FunctorScene(const std_func_scene& f) { FunctorHolder<std_func_scene>::functor = f; }
    };

    struct ECSSystemCI {
        FunctorUpdate        onUpdateFunction;
        FunctorComponent     onComponentAddedToEntityFunction;
        FunctorEntity        onEntityAddedToSceneFunction;
        FunctorScene         onSceneEnteredFunction;
        FunctorScene         onSceneLeftFunction;

        ECSSystemCI()                                        = default;
        virtual ~ECSSystemCI()                               = default;
        ECSSystemCI(const ECSSystemCI&)                      = delete;
        ECSSystemCI& operator=(const ECSSystemCI&)           = delete;
        ECSSystemCI(ECSSystemCI&& other) noexcept            = delete;
        ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete;

        template<class T> void setUpdateFunction(const T& functor) { 
            onUpdateFunction = std::move(FunctorUpdate(functor));
        }
        template<class T> void setOnComponentAddedToEntityFunction(const T& functor) { 
            onComponentAddedToEntityFunction = std::move(FunctorComponent(functor)); 
        }
        template<class T> void setOnEntityAddedToSceneFunction(const T& functor) { 
            onEntityAddedToSceneFunction = std::move(FunctorEntity(functor)); 
        }
        template<class T> void setOnSceneEnteredFunction(const T& functor) { 
            onSceneEnteredFunction = std::move(FunctorScene(functor)); 
        }
        template<class T> void setOnSceneLeftFunction(const T& functor) { 
            onSceneLeftFunction = std::move(FunctorScene(functor)); 
        }
    };

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