#pragma once
#ifndef ENGINE_ECS_SYSTEM_BASE_CLASS_H
#define ENGINE_ECS_SYSTEM_BASE_CLASS_H

namespace Engine::priv {
    class sparse_set_base;
    class ECS;
}

#include <vector>
#include <tuple>
#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/threading/ThreadingModule.h>

enum class SystemExecutionPolicy : uint32_t {
    Normal = 0,
    Parallel,
    ParallelWait,
};

class SystemBaseClass {
    using ComponentPoolContainer = std::vector<std::pair<uint32_t, Engine::priv::sparse_set_base*>>;
    using FuncUpdate = void(*)(SystemBaseClass&, const float dt, Scene&);
    using FuncComponentAddedToEntity = void(*)(SystemBaseClass&, void* component, Entity);
    using FuncComponentRemovedFromEntity = void(*)(SystemBaseClass&, Entity);
    using FuncEntityAddedToScene = void(*)(SystemBaseClass&, Scene&, Entity);
    using FuncSceneEntered = void(*)(SystemBaseClass&, Scene&);
    using FuncSceneLeft = void(*)(SystemBaseClass&, Scene&);
    private:
        bool hasEntity(Entity entity) const noexcept;
        bool hasAssociatedComponent(uint32_t typeID) noexcept;
        void associateComponentImpl(uint32_t typeID);
        void sortEntities() noexcept;
        void insertionSort(std::vector<Entity>& container);
    protected:
        Engine::priv::ECS&             m_ECS;
        std::vector<Entity>            m_Entities;
        ComponentPoolContainer         m_Components;

        FuncUpdate                     m_UpdateFunction                     = [](SystemBaseClass&, const float dt, Scene&) {};
        FuncComponentAddedToEntity     m_ComponentAddedToEntityFunction     = [](SystemBaseClass&, void*, Entity) {};
        FuncComponentRemovedFromEntity m_ComponentRemovedFromEntityFunction = [](SystemBaseClass&, Entity) {};
        FuncEntityAddedToScene         m_EntityToSceneFunction              = [](SystemBaseClass&, Scene&, Entity) {};
        FuncSceneEntered               m_SceneEnteredFunction               = [](SystemBaseClass&, Scene&) {};
        FuncSceneLeft                  m_SceneLeftFunction                  = [](SystemBaseClass&, Scene&) {};
    public:
        SystemBaseClass(Engine::priv::ECS& ecs)
            : m_ECS { ecs }
        {}

        void onUpdate(const float dt, Scene& scene) noexcept;
        void onEntityAddedToScene(Scene& scene, Entity entity) noexcept;
        void onComponentAddedToEntity(void* component, Entity entity) noexcept;
        void onComponentRemovedFromEntity(Entity entity) noexcept;
        void onSceneEntered(Scene& scene) noexcept;
        void onSceneLeft(Scene& scene) noexcept;

        void addEntity(Entity entity);
        void removeEntity(Entity entity);
        void eraseEntity(std::vector<Entity>& vec, Entity entity);

        template<class T> void setUpdateFunction(T&& func) noexcept;
        template<class T> void setUpdateFunction(const T& func) noexcept;

        template<class T> void setComponentAddedToEntityFunction(T&& func) noexcept;
        template<class T> void setComponentAddedToEntityFunction(const T& func) noexcept;

        template<class T> void setComponentRemovedFromEntityFunction(T&& func) noexcept;
        template<class T> void setComponentRemovedFromEntityFunction(const T& func) noexcept;

        template<class T> void setEntityAddedToSceneFunction(T&& func) noexcept;
        template<class T> void setEntityAddedToSceneFunction(const T& func) noexcept;

        template<class T> void setSceneEnteredFunction(T&& func) noexcept;
        template<class T> void setSceneEnteredFunction(const T& func) noexcept;

        template<class T> void setSceneLeftFunction(T&& func) noexcept;
        template<class T> void setSceneLeftFunction(const T& func) noexcept;

        template<class COMPONENT> void associateComponent();
        template<class COMPONENT> [[nodiscard]] bool hasAssociatedComponent() noexcept;
        template<class COMPONENT> [[nodiscard]] Engine::priv::ECSComponentPool<COMPONENT>& getComponentPool(uint32_t index) noexcept;
};

template<class SYSTEM, class ... COMPONENTS>
class SystemCRTP : public SystemBaseClass {
    private:
        template<class COMPONENT>
        void buildTupleImpl(Entity entity, uint32_t& idx) {
            COMPONENT* ret = getComponentPool(idx).getComponent<COMPONENT>(entity);
            ++idx;
            return ret;
        }

        template<class T>
        std::tuple<> buildTuple(T args, Entity entity) {
            uint32_t idx = 0;
            return std::make_tuple(args, entity, buildTupleImpl<COMPONENTS>(entity, idx));
        }

        template<class T, class F1, class F2>
        void forEachImpl(T args, F1 f1, F2 f2, SystemExecutionPolicy execPolicy) {
            if (m_Entities.size() < 100 || execPolicy == SystemExecutionPolicy::Normal) {
                for (const auto entity : m_Entities) {
                    auto tuple = (this->*f2(args, entity));
                    std::apply(f1, std::move(tuple));
                }
            }else{
                //TODO: implement

                if (execPolicy == SystemExecutionPolicy::ParallelWait) {
                    Engine::priv::threading::waitForAll();
                }
            }
        }
    public:
        static inline uint32_t TYPE_ID = 0;
    public:
        SystemCRTP(Engine::priv::ECS& ecs)
            : SystemBaseClass{ ecs }
        {}

        template<class T>
        void forEach(void(*func)(T, Entity, COMPONENTS*...), T args, SystemExecutionPolicy execPolicy = SystemExecutionPolicy::Normal) {
            using F = std::tuple<T, Entity, COMPONENTS*...>;
            forEachImpl(args, func, &SystemCRTP::buildTuple, execPolicy);
        }
        template<class T>
        void forEach(void(*func)(T, COMPONENTS*...), T args, SystemExecutionPolicy execPolicy = SystemExecutionPolicy::Normal) {
            using F = std::tuple<T, COMPONENTS*...>;
            forEachImpl(args, func, &SystemCRTP::buildTupleNoEntity, execPolicy);
        }
};

#include <serenity/ecs/systems/SystemBaseClass.inl>

#endif