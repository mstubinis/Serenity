#pragma once
#ifndef ENGINE_ECS_SYSTEM_H
#define ENGINE_ECS_SYSTEM_H

#include <vector>
#include <boost/function.hpp>

typedef boost::function<void()>                boost_void_func;

namespace Engine {
    namespace epriv {

        template<typename...> class ECSSystem;

        template <typename TEntity> class ECSSystem<TEntity> {
            protected:
                boost_void_func updateFunction, 
                                onComponentAddedToEntityFunction, 
                                onEntityAddedToSceneFunction, 
                                onSceneChangedFunction;
            public:
                ECSSystem() = default;
                virtual ~ECSSystem() = default;
                ECSSystem(const ECSSystem&) = delete;            // non construction-copyable
                ECSSystem& operator=(const ECSSystem&) = delete; // non copyable

                virtual void update(const float& dt){ updateFunction(); }
                virtual void onComponentAddedToEntity(const float& dt) { onComponentAddedToEntityFunction(); }
                virtual void onEntityAddedToScene(const float& dt) { onEntityAddedToSceneFunction(); }
                virtual void onSceneChanged(const float& dt) { onSceneChangedFunction(); }

                template<typename T, typename ... ARGS> void setUpdateFunction(T& functor, ARGS&&... _args) {
                    updateFunction = boost::bind<void>(functor, std::forward<ARGS>(_args)...);
                }
                template<typename T, typename ... ARGS> void setOnComponentAddedToEntityFunction(T& functor, ARGS&&... _args) {
                    onComponentAddedToEntityFunction = boost::bind<void>(functor, std::forward<ARGS>(_args)...);
                }
                template<typename T, typename ... ARGS> void setOnEntityAddedToSceneFunctionFunction(T& functor, ARGS&&... _args) {
                    onEntityAddedToSceneFunction = boost::bind<void>(functor, std::forward<ARGS>(_args)...);
                }
                template<typename T, typename ... ARGS> void setOnSceneChangedFunctionFunction(T& functor, ARGS&&... _args) {
                    onSceneChangedFunction = boost::bind<void>(functor, std::forward<ARGS>(_args)...);
                }
        };
        template <typename TEntity, typename TComponent> class ECSSystem<TEntity, TComponent> : public ECSSystem<TEntity> {
            using super = ECSSystem<TEntity>;
            protected:
                ECSComponentPool<TEntity, TComponent>& componentPool;
            public:
                ECSSystem(ECSComponentPool<TEntity, TComponent>& _componentPool):componentPool(_componentPool){
                }
                virtual ~ECSSystem() {

                }
        };
    };
};
#endif