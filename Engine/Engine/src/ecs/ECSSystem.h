#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>
#include "ecs/ECS.h"

typedef boost::function<void()>                boost_void_func;
typedef boost::function<void(const float&)>    boost_update_func;


namespace Engine {
    namespace epriv {
        struct ECSSystemCI final {
            boost_update_func   updateFunction;
            boost_void_func     onComponentAddedToEntityFunction, onEntityAddedToSceneFunction, onSceneChangedFunction;

            ECSSystemCI() = default;
            ECSSystemCI(boost_update_func& _update, boost_void_func& _onCAddedToE, boost_void_func& _onEAddedToS, boost_void_func& _onSChanged) {
                updateFunction                    = _update;
                onComponentAddedToEntityFunction  = _onCAddedToE;
                onEntityAddedToSceneFunction      = _onEAddedToS;
                onSceneChangedFunction            = _onSChanged;
            }
            ~ECSSystemCI() = default;
            ECSSystemCI(const ECSSystemCI&) = delete;                      // non construction-copyable
            ECSSystemCI& operator=(const ECSSystemCI&) = delete;           // non copyable
            ECSSystemCI(ECSSystemCI&& other) noexcept = delete;            // non construction-moveable
            ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete; // non moveable

        };


        template<typename...> class ECSSystemBase;
        template<typename...> class ECSSystem;
        template <typename TEntity> class ECSSystemBase<TEntity> {
            protected:
                boost_update_func updateFunction;
                boost_void_func   onComponentAddedToEntityFunction,
                                  onEntityAddedToSceneFunction, 
                                  onSceneChangedFunction;
            public:
                ECSSystemBase() = default;
                virtual ~ECSSystemBase() = default;
                ECSSystemBase(const ECSSystemBase&) = delete;                      // non construction-copyable
                ECSSystemBase& operator=(const ECSSystemBase&) = delete;           // non copyable
                ECSSystemBase(ECSSystemBase&& other) noexcept = delete;            // non construction-moveable
                ECSSystemBase& operator=(ECSSystemBase&& other) noexcept = delete; // non moveable

                virtual void update(const float& dt){ updateFunction(dt); }
                virtual void onComponentAddedToEntity(const float& dt) { onComponentAddedToEntityFunction(); }
                virtual void onEntityAddedToScene(const float& dt) { onEntityAddedToSceneFunction(); }
                virtual void onSceneChanged(const float& dt) { onSceneChangedFunction(); }

                template<typename T> void setUpdateFunction(T& functor) { updateFunction = boost::bind<void>(functor, this, 0.01666666f); }
                template<typename T> void setOnComponentAddedToEntityFunction(T& functor) { onComponentAddedToEntityFunction = boost::bind<void>(functor, this); }
                template<typename T> void setOnEntityAddedToSceneFunctionFunction(T& functor) { onEntityAddedToSceneFunction = boost::bind<void>(functor, this); }
                template<typename T> void setOnSceneChangedFunctionFunction(T& functor) { onSceneChangedFunction = boost::bind<void>(functor, this); }
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> final : public ECSSystemBase<TEntity> {
            using super     = ECSSystemBase<TEntity>;
            using CPoolType = ECSComponentPool<TEntity, TComponent>;
            private:
                CPoolType& componentPool;
            public:
                ECSSystem() = delete;
                ECSSystem(ECSSystemCI& _systemCI, ECS<TEntity>& _ecs):componentPool(_ecs.template getPool<TComponent>()){
                    super::updateFunction                   = _systemCI.updateFunction;
                    super::onComponentAddedToEntityFunction = _systemCI.onComponentAddedToEntityFunction;
                    super::onEntityAddedToSceneFunction     = _systemCI.onEntityAddedToSceneFunction;
                    super::onSceneChangedFunction           = _systemCI.onSceneChangedFunction;
                }
                ~ECSSystem() = default;

                ECSSystem(const ECSSystem&) = delete;                      // non construction-copyable
                ECSSystem& operator=(const ECSSystem&) = delete;           // non copyable
                ECSSystem(ECSSystem&& other) noexcept = delete;            // non construction-moveable
                ECSSystem& operator=(ECSSystem&& other) noexcept = delete; // non moveable
        };
    };
};
#endif