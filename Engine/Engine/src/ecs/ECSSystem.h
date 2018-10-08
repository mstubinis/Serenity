#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include "ecs/ECS.h"

#include <iostream>

struct Entity;
class Scene;
namespace Engine {
    namespace epriv {
        template<typename ...> class ECSSystemBase;
        template<typename ...> class ECSSystem;

        typedef boost::function<void(void*, const float&)>  func_update;
        typedef boost::function<void(void*, Entity&)>       func_entity;
        typedef boost::function<void(void*)>                func_component;
        typedef boost::function<void(void*,Scene&)>         func_scene;

        template<typename T> struct FunctorHolder {
            T functor;
            FunctorHolder() = default;
            FunctorHolder(const T& _f) { functor = _f; }
            virtual ~FunctorHolder() = default;
        };
        struct FunctorUpdate final : public FunctorHolder<func_update>{
            FunctorUpdate(){} FunctorUpdate(const func_update& _f) { FunctorHolder<func_update>::functor = _f; }
        };
        struct FunctorEntity final : public FunctorHolder<func_entity> {
            FunctorEntity() {} FunctorEntity(const func_entity& _f) { FunctorHolder<func_entity>::functor = _f; }
        };
        struct FunctorComponent : public FunctorHolder<func_component> {
            FunctorComponent() {} FunctorComponent(const func_component& _f) { FunctorHolder<func_component>::functor = _f; }
        };
        struct FunctorScene final : public FunctorHolder<func_scene> {
            FunctorScene() {} FunctorScene(const func_scene& _f) { FunctorHolder<func_scene>::functor = _f; }
        };

        struct ECSSystemCI {
            FunctorUpdate        updateFunction;
            FunctorComponent     onComponentAddedToEntityFunction;
            FunctorEntity        onEntityAddedToSceneFunction;
            FunctorScene         onSceneEnteredFunction;
            FunctorScene         onSceneLeftFunction;
            ECSSystemCI() = default;
            virtual ~ECSSystemCI() = default;
            ECSSystemCI(const ECSSystemCI&) = delete;                      // non construction-copyable
            ECSSystemCI& operator=(const ECSSystemCI&) = delete;           // non copyable
            ECSSystemCI(ECSSystemCI&& other) noexcept = delete;            // non construction-moveable
            ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete; // non moveable

            template<typename T> void setUpdateFunction(const T& functor) {
                updateFunction = FunctorUpdate(functor);
            }
            template<typename T> void setOnComponentAddedToEntityFunction(const T& functor) {
                onComponentAddedToEntityFunction = FunctorComponent(functor);
            }
            template<typename T> void setOnEntityAddedToSceneFunction(const T& functor) {
                onEntityAddedToSceneFunction = FunctorEntity(functor);
            }
            template<typename T> void setOnSceneEnteredFunction(const T& functor) {
                onSceneEnteredFunction = FunctorScene(functor);
            }
            template<typename T> void setOnSceneLeftFunction(const T& functor) {
                onSceneLeftFunction = FunctorScene(functor);
            }
        };


        template <typename TEntity> class ECSSystemBase<TEntity> {
            protected:
                func_update         _SUF;
                func_component      _CAE;
                func_entity         _EAS;
                func_scene          _SEF;
                func_scene          _SLF;
            public:
                ECSSystemBase() = default;
                virtual ~ECSSystemBase() = default;
                ECSSystemBase(const ECSSystemBase&) = delete;                      // non construction-copyable
                ECSSystemBase& operator=(const ECSSystemBase&) = delete;           // non copyable
                ECSSystemBase(ECSSystemBase&& other) noexcept = delete;            // non construction-moveable
                ECSSystemBase& operator=(ECSSystemBase&& other) noexcept = delete; // non moveable

                virtual void update(const float& dt) {}
                virtual void onComponentAddedToEntity(void*) {}
                virtual void onEntityAddedToScene(Entity&) {}
                virtual void onSceneLeft(Scene& _Scene) {}
                virtual void onSceneEntered(Scene& _Scene) {}
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> final : public ECSSystemBase<TEntity> {
            using super     = ECSSystemBase<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;

                void _SUF(const FunctorUpdate& _functor) {
                    super::_SUF = boost::bind(_functor.functor, (void*)&componentPool, _2);
                }
                void _CAE(const FunctorComponent& _functor) {
                    super::_CAE = boost::bind(_functor.functor, _1);
                }
                void _EAS(const FunctorEntity& _functor) {
                    super::_EAS = boost::bind(_functor.functor, (void*)&componentPool, _2);
                }
                void _SEF(const FunctorScene& _functor) {
                    super::_SEF = boost::bind(_functor.functor, (void*)&componentPool, _2);
                }
                void _SLF(const FunctorScene& _functor) {
                    super::_SLF = boost::bind(_functor.functor, (void*)&componentPool, _2);
                }
            public:
                ECSSystem() = delete;
                ECSSystem(const ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
                    _SUF(_systemCI.updateFunction);
                    _CAE(_systemCI.onComponentAddedToEntityFunction);
                    _EAS(_systemCI.onEntityAddedToSceneFunction);
                    _SEF(_systemCI.onSceneEnteredFunction);
                    _SLF(_systemCI.onSceneLeftFunction);
                }
                ~ECSSystem() = default;

                ECSSystem(const ECSSystem&) = delete;                      // non construction-copyable
                ECSSystem& operator=(const ECSSystem&) = delete;           // non copyable
                ECSSystem(ECSSystem&& other) noexcept = delete;            // non construction-moveable
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete; // non moveable


                void update(const float& dt) { 
                    super::_SUF((void*)&componentPool,dt);
                }
                void onComponentAddedToEntity(void* _component) {
                    super::_CAE(_component);
                }
                void onEntityAddedToScene(Entity& _entity) { 
                    super::_EAS((void*)&componentPool, _entity);
                }
                void onSceneEntered(Scene& _Scene) {
                    super::_SEF((void*)&componentPool, _Scene);
                }
                void onSceneLeft(Scene& _Scene) {
                    super::_SLF((void*)&componentPool, _Scene);
                }
        };
    };
};
#endif