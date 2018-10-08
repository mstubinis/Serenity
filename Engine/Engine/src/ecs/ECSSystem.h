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
        typedef boost::function<void(void*,Scene&,Scene&)>  func_scene;

        template<typename T> struct FunctorHolder {
            T functor;
            FunctorHolder() = default;
            FunctorHolder(const T& _f) { functor = _f; }
            virtual ~FunctorHolder() = default;
        };
        struct FunctorHolderUpdate final : public FunctorHolder<func_update>{
            FunctorHolderUpdate(){} FunctorHolderUpdate(const func_update& _f) { FunctorHolder<func_update>::functor = _f; }
        };
        struct FunctorHolderEntity final : public FunctorHolder<func_entity> {
            FunctorHolderEntity() {} FunctorHolderEntity(const func_entity& _f) { FunctorHolder<func_entity>::functor = _f; }
        };
        struct FunctorHolderComponent : public FunctorHolder<func_component> {
            FunctorHolderComponent() {} FunctorHolderComponent(const func_component& _f) { FunctorHolder<func_component>::functor = _f; }
        };
        struct FunctorHolderScene final : public FunctorHolder<func_scene> {
            FunctorHolderScene() {} FunctorHolderScene(const func_scene& _f) { FunctorHolder<func_scene>::functor = _f; }
        };

        struct ECSSystemCI {
            FunctorHolderUpdate        updateFunction;
            FunctorHolderComponent     onComponentAddedToEntityFunction;
            FunctorHolderEntity        onEntityAddedToSceneFunction;
            FunctorHolderScene         onSceneChangedFunction;

            ECSSystemCI() {
                updateFunction = FunctorHolderUpdate();
                onComponentAddedToEntityFunction = FunctorHolderComponent();
                onEntityAddedToSceneFunction = FunctorHolderEntity();
                onSceneChangedFunction = FunctorHolderScene();
            }
            virtual ~ECSSystemCI() = default;
            ECSSystemCI(const ECSSystemCI&) = delete;                      // non construction-copyable
            ECSSystemCI& operator=(const ECSSystemCI&) = delete;           // non copyable
            ECSSystemCI(ECSSystemCI&& other) noexcept = delete;            // non construction-moveable
            ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete; // non moveable

            template<typename T> void setUpdateFunction(const T& functor) {
                updateFunction = FunctorHolderUpdate(functor);
            }
            template<typename T> void setOnComponentAddedToEntityFunction(const T& functor) {
                onComponentAddedToEntityFunction = FunctorHolderComponent(functor);
            }
            template<typename T> void setOnEntityAddedToSceneFunctionFunction(const T& functor) {
                onEntityAddedToSceneFunction = FunctorHolderEntity(functor);
            }
            template<typename T> void setOnSceneChangedFunctionFunction(const T& functor) {
                onSceneChangedFunction = FunctorHolderScene(functor);
            }
        };


        template <typename TEntity> class ECSSystemBase<TEntity> {
            protected:
                func_update         updateFunction;
                func_component      onComponentAddedToEntityFunction;
                func_entity         onEntityAddedToSceneFunction;
                func_scene          onSceneChangedFunction;
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
                virtual void onSceneChanged(Scene& _oldScene, Scene& _newScene) {}
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> final : public ECSSystemBase<TEntity> {
            using super     = ECSSystemBase<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;
            public:
                ECSSystem() = delete;
                ECSSystem(const ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
                    setUpdateFunction(_systemCI.updateFunction);
                    setOnComponentAddedToEntityFunction(_systemCI.onComponentAddedToEntityFunction);
                    setOnEntityAddedToSceneFunctionFunction(_systemCI.onEntityAddedToSceneFunction);
                    setOnSceneChangedFunctionFunction(_systemCI.onSceneChangedFunction);
                }
                ~ECSSystem() = default;

                ECSSystem(const ECSSystem&) = delete;                      // non construction-copyable
                ECSSystem& operator=(const ECSSystem&) = delete;           // non copyable
                ECSSystem(ECSSystem&& other) noexcept = delete;            // non construction-moveable
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete; // non moveable

                void setUpdateFunction(const FunctorHolderUpdate& _functor) {
                    super::updateFunction = boost::bind(_functor.functor, (void*)&componentPool,_2);
                }
                void setOnComponentAddedToEntityFunction(const FunctorHolderComponent& _functor) {
                    super::onComponentAddedToEntityFunction = boost::bind(_functor.functor,_1);
                }
                void setOnEntityAddedToSceneFunctionFunction(const FunctorHolderEntity& _functor) {
                    super::onEntityAddedToSceneFunction = boost::bind(_functor.functor, (void*)&componentPool,_2);
                }
                void setOnSceneChangedFunctionFunction(const FunctorHolderScene& _functor) {
                    super::onSceneChangedFunction = boost::bind(_functor.functor, (void*)&componentPool,_2,_3);
                }

                void update(const float& dt) { 
                    super::updateFunction((void*)&componentPool,dt);
                }
                void onComponentAddedToEntity(void* _component) {
                    super::onComponentAddedToEntityFunction(_component);
                }
                void onEntityAddedToScene(Entity& _entity) { 
                    super::onEntityAddedToSceneFunction((void*)&componentPool, _entity);
                }
                void onSceneChanged(Scene& _oldScene, Scene& _newScene) {
                    super::onSceneChangedFunction((void*)&componentPool, _oldScene, _newScene);
                }
        };
    };
};
#endif