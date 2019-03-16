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
        template<typename ...> class ECSSystem;

        struct FunctorUpdateEmpty final { 
            void operator()(void* _cPool, const float& dt, Scene&) const { } };
        struct FunctorComponentEmpty final { 
            void operator()(void* _compt, Entity&) const { } };
        struct FunctorEntityEmpty final { 
            void operator()(void* _cPool, Entity&, Scene&) const { } };
        struct FunctorSceneEmpty final { 
            void operator()(void* _cPool, Scene&) const { } };

        typedef boost::function<void(void*, const float&, Scene&)>  func_update;
        typedef boost::function<void(void*, Entity&, Scene&)>       func_entity;
        typedef boost::function<void(void*, Entity&)>               func_component;
        typedef boost::function<void(void*, Scene&)>                func_scene;

        template<class T> struct FunctorHolder {
            T functor;
            FunctorHolder() = default;
            FunctorHolder(const T& _f) { functor = _f; }
            virtual ~FunctorHolder() = default;
        };
        struct FunctorUpdate final : public FunctorHolder<func_update>{
            FunctorUpdate() { FunctorHolder<func_update>::functor = FunctorUpdateEmpty(); }
            FunctorUpdate(const func_update& _f) { FunctorHolder<func_update>::functor = _f; }
        };
        struct FunctorComponent : public FunctorHolder<func_component> {
            FunctorComponent() { FunctorHolder<func_component>::functor = FunctorComponentEmpty(); }
            FunctorComponent(const func_component& _f) { FunctorHolder<func_component>::functor = _f; }
        };
        struct FunctorEntity final : public FunctorHolder<func_entity> {
            FunctorEntity() { FunctorHolder<func_entity>::functor = FunctorEntityEmpty(); }
            FunctorEntity(const func_entity& _f) { FunctorHolder<func_entity>::functor = _f; }
        };
        struct FunctorScene final : public FunctorHolder<func_scene> {
            FunctorScene() { FunctorHolder<func_scene>::functor = FunctorSceneEmpty(); }
            FunctorScene(const func_scene& _f) { FunctorHolder<func_scene>::functor = _f; }
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

                virtual void update(const float& dt, Scene&) {}
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

                void Bind_SUF(const FunctorUpdate& _f) { 
                    super::_SUF = boost::bind(_f.functor, _1, _2, _3); }
                void Bind_CAE(const FunctorComponent& _f) {
                    super::_CAE = boost::bind(_f.functor, _1, _2); }
                void Bind_EAS(const FunctorEntity& _f) {
                    super::_EAS = boost::bind(_f.functor, _1, _2, _3); }
                void Bind_SEF(const FunctorScene& _f) {
                    super::_SEF = boost::bind(_f.functor, _1, _2); }
                void Bind_SLF(const FunctorScene& _f) {
                    super::_SLF = boost::bind(_f.functor, _1, _2); }
            public:
                ECSSystem() = default;
                ECSSystem(const ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
                    Bind_SUF(_systemCI.updateFunction);
                    Bind_CAE(_systemCI.onComponentAddedToEntityFunction);
                    Bind_EAS(_systemCI.onEntityAddedToSceneFunction);
                    Bind_SEF(_systemCI.onSceneEnteredFunction);
                    Bind_SLF(_systemCI.onSceneLeftFunction);
                }
                ~ECSSystem() = default;

                ECSSystem(const ECSSystem&) = delete;
                ECSSystem& operator=(const ECSSystem&) = delete;
                ECSSystem(ECSSystem&& other) noexcept = delete;
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete;

                void update(const float& dt, Scene& _scene) { 
                    super::_SUF(&componentPool, dt, _scene); }
                void onComponentAddedToEntity(void* _component, TEntity& _entity) { 
                    super::_CAE(_component, _entity); }
                void onEntityAddedToScene(TEntity& _entity, Scene& _scene) { 
                    super::_EAS(&componentPool, _entity, _scene); }
                void onSceneEntered(Scene& _scene) { 
                    super::_SEF(&componentPool, _scene); }
                void onSceneLeft(Scene& _scene) { 
                    super::_SLF(&componentPool, _scene); }
        };
    };
};
#endif