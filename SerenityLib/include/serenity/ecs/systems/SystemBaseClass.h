#pragma once
#ifndef ENGINE_ECS_SYSTEM_BASE_CLASS_H
#define ENGINE_ECS_SYSTEM_BASE_CLASS_H

class  Scene;
namespace Engine::priv {
    class sparse_set_base;
    class ECS;
}

#include <cstdint>
#include <vector>
#include <tuple>
#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/threading/ThreadingModule.h>

enum class SystemExecutionPolicy : uint32_t {
    Normal = 0,   //Linear execution
    Parallel,     //Parallel execution using built in engine thread pool. main thread will NOT wait for the execution to finish
    ParallelWait, //Parallel execution using built in engine thread pool. main thread WILL WAIT until the execution is finished
};

class SystemBaseClass {
    using ComponentPoolContainer         = std::vector<std::pair<uint32_t, Engine::priv::sparse_set_base*>>;
    using FuncUpdate                     = void(*)(SystemBaseClass&, const float dt, Scene&);
    using FuncComponentAddedToEntity     = void(*)(SystemBaseClass&, void* component, Entity);
    using FuncComponentRemovedFromEntity = void(*)(SystemBaseClass&, Entity);
    using FuncEntityAddedToScene         = void(*)(SystemBaseClass&, Scene&, Entity);
    using FuncSceneEntered               = void(*)(SystemBaseClass&, Scene&);
    using FuncSceneLeft                  = void(*)(SystemBaseClass&, Scene&);
    private:
        bool hasEntity(Entity) const noexcept;
        [[nodiscard]] size_t getEntityIdxInContainer(Entity) const noexcept;
        bool hasAssociatedComponent(uint32_t typeID) noexcept;
        void associateComponentImpl(uint32_t typeID);
        void sortEntities() noexcept;
    protected:
        Engine::priv::ECS&             m_ECS;
        std::vector<Entity>            m_Entities;
        ComponentPoolContainer         m_AssociatedComponents;

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

        void onUpdate(const float dt, Scene&) noexcept;
        void onEntityAddedToScene(Scene&, Entity) noexcept;
        void onComponentAddedToEntity(void* component, Entity) noexcept;
        void onComponentRemovedFromEntity(Entity) noexcept;
        void onSceneEntered(Scene&) noexcept;
        void onSceneLeft(Scene&) noexcept;

        [[nodiscard]] inline Engine::priv::ECS& getECS() noexcept { return m_ECS; }
        [[nodiscard]] Entity getEntity(uint32_t entityID) const noexcept;

        void addEntity(Entity) noexcept;
        void removeEntity(Entity) noexcept;
        bool eraseEntity(Entity);

        template<class FUNC> void setUpdateFunction(FUNC&& func) noexcept;
        template<class FUNC> void setComponentAddedToEntityFunction(FUNC&& func) noexcept;
        template<class FUNC> void setComponentRemovedFromEntityFunction(FUNC&& func) noexcept;
        template<class FUNC> void setEntityAddedToSceneFunction(FUNC&& func) noexcept;
        template<class FUNC> void setSceneEnteredFunction(FUNC&& func) noexcept;
        template<class FUNC> void setSceneLeftFunction(FUNC&& func) noexcept;

        template<class COMPONENT> void associateComponent();
        template<class COMPONENT> [[nodiscard]] bool hasAssociatedComponent() noexcept;
        template<class COMPONENT> [[nodiscard]] Engine::priv::ECSComponentPool<COMPONENT>& getComponentPool(uint32_t index) noexcept;
};

template<class SYSTEM, class ... COMPONENTS>
class SystemCRTP : public SystemBaseClass {
    public:
        static inline uint32_t TYPE_ID = std::numeric_limits<uint32_t>().max();
    private:
        template<class COMPONENT>
        COMPONENT* buildTupleImpl(Entity entity, uint32_t& idx) {
            auto& pool     = getComponentPool<COMPONENT>(idx);
            COMPONENT* ret = pool.getComponent(entity);
            ++idx;
            return ret;
        }
        template<class T>
        std::tuple<T, Entity, COMPONENTS*...> buildTuple(T args, Entity entity) noexcept {
            uint32_t idx = 0; 
            return std::tuple<T, Entity, COMPONENTS*...>{ args, entity, buildTupleImpl<COMPONENTS>(entity, idx)... }; //this MUST use brace-initialization as it forces left to right parameter pack processing
        }
        std::tuple<Entity, COMPONENTS*...> buildTuple(Entity entity) noexcept {
            uint32_t idx = 0; 
            return std::tuple<Entity, COMPONENTS*...>{ entity, buildTupleImpl<COMPONENTS>(entity, idx)... }; //this MUST use brace-initialization as it forces left to right parameter pack processing
        }

        template<class TArgStruct, class F1, class F2>
        void forEachImpl(TArgStruct argStruct, F1 f1, F2 f2, SystemExecutionPolicy execPolicy) {
            if (m_Entities.size() < 100 || execPolicy == SystemExecutionPolicy::Normal) {
                for (const auto entity : m_Entities) {
                    std::apply(  f1, (this->*f2)(argStruct, entity) );
                }
            }else{
                auto pairs = Engine::splitVectorPairs(m_Entities);
                for (size_t k = 0U; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &f2, &f1, &argStruct, this]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            std::apply(  f1, (this->*f2)(argStruct, m_Entities[j]) );
                        }
                    });
                }
                if (execPolicy == SystemExecutionPolicy::ParallelWait) {
                    Engine::priv::threading::waitForAll();
                }
            }
        }
        template<class F1, class F2>
        void forEachImpl(F1 f1, F2 f2, SystemExecutionPolicy execPolicy) {
            if (m_Entities.size() < 100 || execPolicy == SystemExecutionPolicy::Normal) {
                for (const auto entity : m_Entities) {
                    std::apply( f1, (this->*f2)(entity) );
                }
            }else{
                auto pairs = Engine::splitVectorPairs(m_Entities);
                for (size_t k = 0U; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &f2, &f1, this]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            std::apply( f1, (this->*f2)(m_Entities[j]) );
                        }
                    });
                }
                if (execPolicy == SystemExecutionPolicy::ParallelWait) {
                    Engine::priv::threading::waitForAll();
                }
            }
        }
    public:
        SystemCRTP(Engine::priv::ECS& ecs)
            : SystemBaseClass{ ecs }
        {}

        template<class TArgStruct>
        inline void forEach(void(*func)(TArgStruct, Entity, COMPONENTS*...), TArgStruct argStruct, SystemExecutionPolicy execPolicy = SystemExecutionPolicy::Normal) noexcept {
            using F = std::tuple<TArgStruct, Entity, COMPONENTS*...>(SystemCRTP::*)(TArgStruct, Entity);
            forEachImpl<TArgStruct, decltype(func), F>(argStruct, func, &SystemCRTP::buildTuple, execPolicy);
        }
        inline void forEach(void(*func)(Entity, COMPONENTS*...), SystemExecutionPolicy execPolicy = SystemExecutionPolicy::Normal) noexcept {
            using F = std::tuple<Entity, COMPONENTS*...>(SystemCRTP::*)(Entity);
            forEachImpl<decltype(func), F>(func, &SystemCRTP::buildTuple, execPolicy);
        }
};

#include <serenity/ecs/systems/SystemBaseClass.inl>

#endif