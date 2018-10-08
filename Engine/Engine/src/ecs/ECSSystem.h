#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include "ecs/ECS.h"

#include <iostream>

namespace Engine {
    namespace epriv {
        template<typename ...> class ECSSystemBase;
        template<typename ...> class ECSSystem;

        typedef boost::function<void(void*, const float&)> func_update;
        typedef boost::function<void(void*)>               func_other;
        typedef boost::function<void()>                    func_void;

        struct updateEmpty final { void operator()(void* componentPool, const float& dt) const {} };
        struct otherEmpty final { void operator()(void* componentPool) const {} };

        struct FunctorHolderUpdate final {
            func_update functor;
            FunctorHolderUpdate() { updateEmpty f; functor = f; }
            FunctorHolderUpdate(const func_update& _functor) :functor(_functor) {}
            ~FunctorHolderUpdate() = default;
        };
        struct FunctorHolderOther final {
            func_other functor;
            FunctorHolderOther() { otherEmpty f; functor = f; }
            FunctorHolderOther(const func_other& _functor):functor(_functor){}
            ~FunctorHolderOther() = default;
        };

        struct ECSSystemCI {
            FunctorHolderUpdate   updateFunction;
            FunctorHolderOther    onComponentAddedToEntityFunction;
            FunctorHolderOther    onEntityAddedToSceneFunction;
            FunctorHolderOther    onSceneChangedFunction;

            ECSSystemCI() {
                updateFunction = FunctorHolderUpdate();
                onComponentAddedToEntityFunction = FunctorHolderOther();
                onEntityAddedToSceneFunction = FunctorHolderOther();
                onSceneChangedFunction = FunctorHolderOther();
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
                onComponentAddedToEntityFunction = FunctorHolderOther(functor);
            }
            template<typename T> void setOnEntityAddedToSceneFunctionFunction(const T& functor) {
                onEntityAddedToSceneFunction = FunctorHolderOther(functor);
            }
            template<typename T> void setOnSceneChangedFunctionFunction(const T& functor) {
                onSceneChangedFunction = FunctorHolderOther(functor);
            }
        };


        template <typename TEntity> class ECSSystemBase<TEntity> {
            protected:
                func_update    updateFunction;
                func_other     onComponentAddedToEntityFunction;
                func_other     onEntityAddedToSceneFunction;
                func_other     onSceneChangedFunction;
            public:
                ECSSystemBase() = default;
                virtual ~ECSSystemBase() = default;
                ECSSystemBase(const ECSSystemBase&) = delete;                      // non construction-copyable
                ECSSystemBase& operator=(const ECSSystemBase&) = delete;           // non copyable
                ECSSystemBase(ECSSystemBase&& other) noexcept = delete;            // non construction-moveable
                ECSSystemBase& operator=(ECSSystemBase&& other) noexcept = delete; // non moveable

                virtual void update(const float& dt) {}
                virtual void onComponentAddedToEntity() {}
                virtual void onEntityAddedToScene() {}
                virtual void onSceneChanged() {}
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
                    super::updateFunction = boost::bind(_functor.functor, (void*)&componentPool,0.0166666666f);
                }
                void setOnComponentAddedToEntityFunction(const FunctorHolderOther& _functor) {
                    super::onComponentAddedToEntityFunction = boost::bind(_functor.functor, (void*)&componentPool);
                }
                void setOnEntityAddedToSceneFunctionFunction(const FunctorHolderOther& _functor) {
                    super::onEntityAddedToSceneFunction = boost::bind(_functor.functor, (void*)&componentPool);
                }
                void setOnSceneChangedFunctionFunction(const FunctorHolderOther& _functor) {
                    super::onSceneChangedFunction = boost::bind(_functor.functor, (void*)&componentPool);
                }

                void update(const float& dt) { 
                    super::updateFunction((void*)&componentPool,dt);
                }
                void onComponentAddedToEntity() { 
                    super::onComponentAddedToEntityFunction((void*)&componentPool);
                }
                void onEntityAddedToScene() { 
                    super::onEntityAddedToSceneFunction((void*)&componentPool);
                }
                void onSceneChanged() { 
                    super::onSceneChangedFunction((void*)&componentPool);
                }
        };
    };
};
#endif