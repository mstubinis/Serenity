#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include <ecs/ECS.h>

#include <iostream>

struct Entity;
class Scene;
namespace Engine {
namespace epriv {
    template<typename ...> class ECSSystem;

    struct FunctorUpdateEmpty final { 
        void operator()(void* cPool, const double& dt, Scene&) const { } };
    struct FunctorComponentEmpty final { 
        void operator()(void* compt, Entity&) const { } };
    struct FunctorEntityEmpty final { 
        void operator()(void* cPool, Entity&, Scene&) const { } };
    struct FunctorSceneEmpty final { 
        void operator()(void* cPool, Scene&) const { } };

    typedef boost::function<void(void*, const double&, Scene&)>  func_update;
    typedef boost::function<void(void*, Entity&, Scene&)>        func_entity;
    typedef boost::function<void(void*, Entity&)>                func_component;
    typedef boost::function<void(void*, Scene&)>                 func_scene;

    template<class T> struct FunctorHolder {
        T functor;
        FunctorHolder() = default;
        FunctorHolder(const T& f) { functor = f; }
        virtual ~FunctorHolder() = default;
    };
    struct FunctorUpdate final : public FunctorHolder<func_update>{
        FunctorUpdate() { FunctorHolder<func_update>::functor = FunctorUpdateEmpty(); }
        FunctorUpdate(const func_update& f) { FunctorHolder<func_update>::functor = f; }
    };
    struct FunctorComponent : public FunctorHolder<func_component> {
        FunctorComponent() { FunctorHolder<func_component>::functor = FunctorComponentEmpty(); }
        FunctorComponent(const func_component& f) { FunctorHolder<func_component>::functor = f; }
    };
    struct FunctorEntity final : public FunctorHolder<func_entity> {
        FunctorEntity() { FunctorHolder<func_entity>::functor = FunctorEntityEmpty(); }
        FunctorEntity(const func_entity& f) { FunctorHolder<func_entity>::functor = f; }
    };
    struct FunctorScene final : public FunctorHolder<func_scene> {
        FunctorScene() { FunctorHolder<func_scene>::functor = FunctorSceneEmpty(); }
        FunctorScene(const func_scene& f) { FunctorHolder<func_scene>::functor = f; }
    };

    struct ECSSystemCI {
        FunctorUpdate        updateFunction;
        FunctorComponent     onComponentAddedToEntityFunction;
        FunctorEntity        onEntityAddedToSceneFunction;
        FunctorScene         onSceneEnteredFunction;
        FunctorScene         onSceneLeftFunction;

        ECSSystemCI() = default;
        virtual ~ECSSystemCI() = default;
        ECSSystemCI(const ECSSystemCI&) = delete;
        ECSSystemCI& operator=(const ECSSystemCI&) = delete;
        ECSSystemCI(ECSSystemCI&& other) noexcept = delete;
        ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete;

        template<class T> void setUpdateFunction(const T& functor) { updateFunction = FunctorUpdate(functor); }
        template<class T> void setOnComponentAddedToEntityFunction(const T& functor) { onComponentAddedToEntityFunction = FunctorComponent(functor); }
        template<class T> void setOnEntityAddedToSceneFunction(const T& functor) { onEntityAddedToSceneFunction = FunctorEntity(functor); }
        template<class T> void setOnSceneEnteredFunction(const T& functor) { onSceneEnteredFunction = FunctorScene(functor); }
        template<class T> void setOnSceneLeftFunction(const T& functor) { onSceneLeftFunction = FunctorScene(functor); }
    };

    template <class TEntity> class ECSSystem<TEntity> {
        protected:
            func_update         _SUF;
            func_component      _CAE;
            func_entity         _EAS;
            func_scene          _SEF;
            func_scene          _SLF;
        public:
            ECSSystem() = default;
            virtual ~ECSSystem() = default;

            virtual void update(const double& dt, Scene&) {}
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
                super::_SUF = boost::bind(f.functor, _1, _2, _3);
			}
            void Bind_CAE(const FunctorComponent& f) {
                super::_CAE = boost::bind(f.functor, _1, _2);
			}
            void Bind_EAS(const FunctorEntity& f) {
                super::_EAS = boost::bind(f.functor, _1, _2, _3);
			}
            void Bind_SEF(const FunctorScene& f) {
                super::_SEF = boost::bind(f.functor, _1, _2);
			}
            void Bind_SLF(const FunctorScene& f) {
                super::_SLF = boost::bind(f.functor, _1, _2);
			}
        public:
            ECSSystem(const ECSSystemCI& systemCI, ECS<TEntity>& ecs):componentPool(ecs.template getPool<TComponent>()){
                Bind_SUF(systemCI.updateFunction);
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

            void update(const double& dt, Scene& scene) { 
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