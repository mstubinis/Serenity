#pragma once
#ifndef ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H_INCLUDE_GUARD
#define ENGINE_ECS_SYSTEM_CONSTRUCTOR_INFO_H_INCLUDE_GUARD

#include <functional>

class  Scene;
struct Entity;

typedef std::function<void(void*, const float&, Scene&)> std_func_update;
typedef std::function<void(void*, Entity&, Scene&)>      std_func_entity;
typedef std::function<void(void*, Entity&)>              std_func_component;
typedef std::function<void(void*, Scene&)>               std_func_scene;

namespace Engine::priv {
    struct FunctorUpdateEmpty final { void operator()(void* cPool, const float& dt, Scene&) const { } };
    struct FunctorComponentEmpty final { void operator()(void* compt, Entity&) const { } };
    struct FunctorEntityEmpty final { void operator()(void* cPool, Entity&, Scene&) const { } };
    struct FunctorSceneEmpty final { void operator()(void* cPool, Scene&) const { } };

    template<class T> struct FunctorHolder {
        T functor;
        FunctorHolder() = default;
        FunctorHolder(const T& f) { functor = f; }

        FunctorHolder(const FunctorHolder&)                      = default;
        FunctorHolder& operator=(const FunctorHolder&)           = default;
        FunctorHolder(FunctorHolder&& other) noexcept            = default;
        FunctorHolder& operator=(FunctorHolder&& other) noexcept = default;

        virtual ~FunctorHolder() = default;
    };
    struct FunctorUpdate final : public FunctorHolder<std_func_update> {
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

        ECSSystemCI() = default;
        virtual ~ECSSystemCI() = default;
        ECSSystemCI(const ECSSystemCI&) = delete;
        ECSSystemCI& operator=(const ECSSystemCI&) = delete;
        ECSSystemCI(ECSSystemCI&& other) noexcept = delete;
        ECSSystemCI& operator=(ECSSystemCI&& other) noexcept = delete;

        template<class T> void setUpdateFunction(const T& functor) {
            onUpdateFunction = FunctorUpdate(functor);
        }
        template<class T> void setOnComponentAddedToEntityFunction(const T& functor) {
            onComponentAddedToEntityFunction = FunctorComponent(functor);
        }
        template<class T> void setOnEntityAddedToSceneFunction(const T& functor) {
            onEntityAddedToSceneFunction = FunctorEntity(functor);
        }
        template<class T> void setOnSceneEnteredFunction(const T& functor) {
            onSceneEnteredFunction = FunctorScene(functor);
        }
        template<class T> void setOnSceneLeftFunction(const T& functor) {
            onSceneLeftFunction = FunctorScene(functor);
        }
    };
};
#endif