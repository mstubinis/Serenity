#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>

typedef boost::function<void()>                boost_void_func;
typedef boost::function<void(const float&)>    boost_update_func;

namespace Engine {
    namespace epriv {

        template<typename...> class ECSSystem;

        template <typename TEntity> class ECSSystem<TEntity> {
            protected:
                boost_update_func updateFunction;
                boost_void_func   onComponentAddedToEntityFunction,
                                  onEntityAddedToSceneFunction, 
                                  onSceneChangedFunction;
            public:
                ECSSystem() = default;
                virtual ~ECSSystem() = default;
                ECSSystem(const ECSSystem&) = delete;            // non construction-copyable
                ECSSystem& operator=(const ECSSystem&) = delete; // non copyable

                virtual void update(const float& dt){ updateFunction(dt); }
                virtual void onComponentAddedToEntity(const float& dt) { onComponentAddedToEntityFunction(); }
                virtual void onEntityAddedToScene(const float& dt) { onEntityAddedToSceneFunction(); }
                virtual void onSceneChanged(const float& dt) { onSceneChangedFunction(); }

                template<typename T> void setUpdateFunction(T& functor) {
                    updateFunction = boost::bind<void>(functor, this, 0.01666666f);
                }
                template<typename T> void setOnComponentAddedToEntityFunction(T& functor) {
                    onComponentAddedToEntityFunction = boost::bind<void>(functor, this);
                }
                template<typename T> void setOnEntityAddedToSceneFunctionFunction(T& functor) {
                    onEntityAddedToSceneFunction = boost::bind<void>(functor, this);
                }
                template<typename T> void setOnSceneChangedFunctionFunction(T& functor) {
                    onSceneChangedFunction = boost::bind<void>(functor, this);
                }
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> : public ECSSystem<TEntity> {
            using super = ECSSystem<TEntity>;
            protected:
                ECSComponentPool<TEntity, TComponent>& componentPool;
            public:
                ECSSystem(ECSComponentPool<TEntity, TComponent>& _componentPool):componentPool(_componentPool){}
                virtual ~ECSSystem() = default;
        };
    };
};
#endif