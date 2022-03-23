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
#include <stop_token>
#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/threading/ThreadingModule.h>

enum class SystemExecutionPolicy : uint32_t {
    Normal = 0,   //Linear execution
    Parallel,     //Parallel execution using built in engine thread pool. main thread will NOT wait for the execution to finish
    ParallelWait, //Parallel execution using built in engine thread pool. main thread WILL WAIT for the execution to finish
};

class SystemBaseClass {
    private:
        struct ComponentPoolData final {
            uint32_t                       id   = std::numeric_limits<uint32_t>().max();
            Engine::priv::sparse_set_base* pool = nullptr;
        };
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
        std::vector<ComponentPoolData> m_AssociatedComponents;

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

//only need virtual to get the proper class name in the editor's system rendering tab
#ifndef ENGINE_PRODUCTION
        virtual ~SystemBaseClass() = default;
#endif

        void onUpdate(const float dt, Scene&) noexcept;
        void onEntityAddedToScene(Scene&, Entity) noexcept;
        void onComponentAddedToEntity(void* component, Entity) noexcept;
        void onComponentRemovedFromEntity(Entity) noexcept;
        void onSceneEntered(Scene&) noexcept;
        void onSceneLeft(Scene&) noexcept;

        [[nodiscard]] inline Engine::priv::ECS& getECS() noexcept { return m_ECS; }
        [[nodiscard]] Entity getEntity(uint32_t entityID) const noexcept;
        [[nodiscard]] inline const auto& getEntities() const noexcept { return m_Entities; };
        [[nodiscard]] inline const auto& getAssociatedComponents() const noexcept { return m_AssociatedComponents; };
        //[[nodiscard]] const char* getAssociatedComponentDebugName(uint32_t index) const { return m_AssociatedComponents[index].pool->getComponentDebugName(); }

        void addEntity(Entity) noexcept;
        void removeEntity(Entity) noexcept;
        bool eraseEntity(Entity);

        template<class FUNC> inline void setUpdateFunction(FUNC&& func) noexcept { m_UpdateFunction = std::forward<FUNC>(func); }
        template<class FUNC> inline void setComponentAddedToEntityFunction(FUNC&& func) noexcept { m_ComponentAddedToEntityFunction = std::forward<FUNC>(func); }
        template<class FUNC> inline void setComponentRemovedFromEntityFunction(FUNC&& func) noexcept { m_ComponentRemovedFromEntityFunction = std::forward<FUNC>(func); }
        template<class FUNC> inline void setEntityAddedToSceneFunction(FUNC&& func) noexcept { m_EntityToSceneFunction = std::forward<FUNC>(func); }
        template<class FUNC> inline void setSceneEnteredFunction(FUNC&& func) noexcept { m_SceneEnteredFunction = std::forward<FUNC>(func); }
        template<class FUNC> inline void setSceneLeftFunction(FUNC&& func) noexcept { m_SceneLeftFunction = std::forward<FUNC>(func); }

        template<class COMPONENT> void associateComponent() {
            ASSERT(COMPONENT::TYPE_ID != std::numeric_limits<uint32_t>().max(), __FUNCTION__ << "(): COMPONENT::TYPE_ID was null! please register this component (component: " << typeid(COMPONENT).name());
            ASSERT(!hasAssociatedComponent<COMPONENT>(), __FUNCTION__ << "(): already associated component: " << typeid(COMPONENT).name());
            associateComponentImpl(COMPONENT::TYPE_ID);
        }
        template<class COMPONENT> [[nodiscard]] inline bool hasAssociatedComponent() noexcept {
            return hasAssociatedComponent(COMPONENT::TYPE_ID);
        }
        template<class COMPONENT> [[nodiscard]] inline Engine::priv::ECSComponentPool<COMPONENT>& getComponentPool(uint32_t index) noexcept {
            return *static_cast<Engine::priv::ECSComponentPool<COMPONENT>*>(m_AssociatedComponents[index].pool);
        }
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
        template<class ARGS_STRUCT>
        std::tuple<ARGS_STRUCT, Entity, COMPONENTS*...> buildTuple(ARGS_STRUCT args, Entity entity) noexcept {
            uint32_t idx = 0; 
            return std::tuple<ARGS_STRUCT, Entity, COMPONENTS*...>{ args, entity, buildTupleImpl<COMPONENTS>(entity, idx)... }; //this MUST use brace-initialization as it forces left to right parameter pack processing
        }
        std::tuple<Entity, COMPONENTS*...> buildTuple(Entity entity) noexcept {
            uint32_t idx = 0; 
            return std::tuple<Entity, COMPONENTS*...>{ entity, buildTupleImpl<COMPONENTS>(entity, idx)... }; //this MUST use brace-initialization as it forces left to right parameter pack processing
        }

        template<class ARGS_STRUCT, class F1, class FUNC_TUPLE_BUILDER>
        void forEachImplRange(size_t start, size_t end, const ARGS_STRUCT& argStruct, F1 f1, FUNC_TUPLE_BUILDER funcTupleBuilder) {
            for (size_t j = start; j < end; ++j) {
                const auto tuple = (this->*funcTupleBuilder)(argStruct, m_Entities[j]);
                std::apply(f1, std::move(tuple));
            }
        }
        template<class F1, class FUNC_TUPLE_BUILDER>
        void forEachImplRangeNoArgs(size_t start, size_t end, F1 f1, FUNC_TUPLE_BUILDER funcTupleBuilder) {
            for (size_t j = start; j < end; ++j) {
                const auto tuple = (this->*funcTupleBuilder)(m_Entities[j]);
                std::apply(f1, std::move(tuple));
            }
        }

        template<class ARGS_STRUCT, class F1, class FUNC_TUPLE_BUILDER>
        void forEachImpl(const ARGS_STRUCT& argStruct, F1 f1, FUNC_TUPLE_BUILDER funcTupleBuilder, SystemExecutionPolicy execPolicy) {
            if (m_Entities.size() < 100 || execPolicy == SystemExecutionPolicy::Normal) {
                forEachImplRange(0, m_Entities.size(), argStruct, f1, funcTupleBuilder);
            } else {
                auto pairs = Engine::splitVectorPairs(m_Entities);
                for (size_t k = 0; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &funcTupleBuilder, &f1, &argStruct, this]() {
                        forEachImplRange(pairs[k].first, pairs[k].second + 1, argStruct, f1, funcTupleBuilder);
                    });
                }
                if (execPolicy == SystemExecutionPolicy::ParallelWait) {
                    Engine::priv::threading::waitForAll();
                }
            }
        }
        template<class F1, class FUNC_TUPLE_BUILDER>
        void forEachImpl(F1 f1, FUNC_TUPLE_BUILDER funcTupleBuilder, SystemExecutionPolicy execPolicy) {
            if (m_Entities.size() < 100 || execPolicy == SystemExecutionPolicy::Normal) {
                forEachImplRangeNoArgs(0, m_Entities.size(), f1, funcTupleBuilder);
            } else {
                auto pairs = Engine::splitVectorPairs(m_Entities);
                for (size_t k = 0; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &funcTupleBuilder, &f1, this]() {
                        forEachImplRangeNoArgs(pairs[k].first, pairs[k].second + 1, f1, funcTupleBuilder);
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


        template<class ARGS_STRUCT>
        inline void forEach(void(*func)(ARGS_STRUCT, Entity, COMPONENTS*...), const ARGS_STRUCT& arg, SystemExecutionPolicy policy = SystemExecutionPolicy::Normal) noexcept {
            using TUPLE              = std::tuple<ARGS_STRUCT, Entity, COMPONENTS*...>;
            using FUNC_TUPLE_BUILDER = TUPLE(SystemCRTP::*)(ARGS_STRUCT, Entity);
            forEachImpl<ARGS_STRUCT, decltype(func), FUNC_TUPLE_BUILDER>(arg, func, &SystemCRTP::buildTuple, policy);
        }
        inline void forEach(void(*func)(Entity, COMPONENTS*...), SystemExecutionPolicy policy = SystemExecutionPolicy::Normal) noexcept {
            using TUPLE              = std::tuple<Entity, COMPONENTS*...>;
            using FUNC_TUPLE_BUILDER = TUPLE(SystemCRTP::*)(Entity);
            forEachImpl<decltype(func), FUNC_TUPLE_BUILDER>(func, &SystemCRTP::buildTuple, policy);
        }
};

#endif