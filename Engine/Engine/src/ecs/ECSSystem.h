#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include "ecs/ECS.h"


namespace Engine {
    namespace epriv {
        template<typename ...> class ECSSystemBase;
        template<typename ...> class ECSSystem;

        typedef boost::function<void(boost::any&, const float&)> func_update;
        typedef boost::function<void(boost::any&)>               func_other;
        typedef boost::function<void()>                          func_void;

        struct updateEmpty final { void operator()(boost::any& componentPool, const float& dt) const {} };
        struct otherEmpty final { void operator()(boost::any& componentPool) const {} };

        struct FunctorHolderUpdate final {
            func_update functor;
            FunctorHolderUpdate() { updateEmpty f; functor = f; }
            FunctorHolderUpdate(const func_update& _functor) :functor(_functor) {}
        };
        struct FunctorHolderOther final {
            func_other functor;
            FunctorHolderOther() { otherEmpty f; functor = f; }
            FunctorHolderOther(const func_other& _functor):functor(_functor){}
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
            ~ECSSystemCI() = default;
            ECSSystemCI(const ECSSystemCI&) = delete;                      // non construction-copyable
            ECSSystemCI& operator=(const ECSSystemCI&) = delete;           // non copyable
            ECSSystemCI(ECSSystemCI&& other) noexcept = delete;            // non construction-moveable
            ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete; // non moveable

            template<typename T> void setUpdateFunction(T& functor) {
                updateFunction = FunctorHolderUpdate(functor);
            }
            template<typename T> void setOnComponentAddedToEntityFunction(T& functor) {
                onComponentAddedToEntityFunction = FunctorHolderOther(functor);
            }
            template<typename T> void setOnEntityAddedToSceneFunctionFunction(T& functor) {
                onEntityAddedToSceneFunction = FunctorHolderOther(functor);
            }
            template<typename T> void setOnSceneChangedFunctionFunction(T& functor) {
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
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> final : public ECSSystemBase<TEntity> {
            using super     = ECSSystemBase<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;
            public:
                ECSSystem() = delete;
                ECSSystem(ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
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

                void setUpdateFunction(FunctorHolderUpdate& _functor) {
                    super::updateFunction = boost::bind(_functor.functor, boost::any(componentPool), 0.01666666f);
                }
                void setOnComponentAddedToEntityFunction(FunctorHolderOther& _functor) {
                    super::onComponentAddedToEntityFunction = boost::bind(_functor.functor, boost::any(componentPool));
                }
                void setOnEntityAddedToSceneFunctionFunction(FunctorHolderOther& _functor) {
                    super::onEntityAddedToSceneFunction = boost::bind(_functor.functor, boost::any(componentPool));
                }
                void setOnSceneChangedFunctionFunction(FunctorHolderOther& _functor) {
                    super::onSceneChangedFunction = boost::bind(_functor.functor, boost::any(componentPool));
                }

                void update(const float& dt) { super::updateFunction(componentPool,dt); }
                void onComponentAddedToEntity() { super::onComponentAddedToEntityFunction(componentPool); }
                void onEntityAddedToScene() { super::onEntityAddedToSceneFunction(componentPool); }
                void onSceneChanged() { super::onSceneChangedFunction(componentPool); }
        };
    };
};
#endif