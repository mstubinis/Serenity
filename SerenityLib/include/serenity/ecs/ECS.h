#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

class  Entity;
struct SceneOptions;
class  SystemBaseClass;
namespace Engine::priv {
    struct PublicScene;
}

#include <serenity/ecs/ECSEntityPool.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/ecs/ECSSystemPool.h>
#include <serenity/scene/SceneOptions.h>
#include <serenity/types/Types.h>
#include <serenity/system/Macros.h>
#include <tuple>
#include <mutex>
#include <vector>

namespace Engine::priv {
    class ECS final {
        friend struct Engine::priv::PublicScene;
        private:
            static inline uint32_t                          m_RegisteredComponents = 0;
            ECSEntityPool                                   m_EntityPool;
            ECSSystemPool                                   m_SystemPool;
            std::vector<Entity>                             m_JustAddedEntities;
            std::vector<Entity>                             m_DestroyedEntities;
            std::vector<std::unique_ptr<sparse_set_base>>   m_ComponentPools;
            std::recursive_mutex                            m_MutexRecursive; //recursive as addComponent() can often be nested
            SceneOptions                                    m_SceneOptions;


            template<class COMPONENT> [[nodiscard]] Engine::view_ptr<COMPONENT> internal_get_component(Entity entity) noexcept {
                return getComponentPool<COMPONENT>().getComponent(entity);
            }
        public:
            ECS() = default;
            ECS(const ECS&)                = delete;
            ECS& operator=(const ECS&)     = delete;
            ECS(ECS&&) noexcept            = delete;
            ECS& operator=(ECS&&) noexcept = delete;

            void init(const SceneOptions&);

            void clearAllEntities() noexcept;

            [[nodiscard]] inline const ECSEntityPool& getEntityPool() const noexcept { return m_EntityPool; }
            [[nodiscard]] Entity createEntity(Scene&);
            [[nodiscard]] inline sparse_set_base* getComponentPool(uint32_t typeID) noexcept { return m_ComponentPools[typeID - 1].get(); }

            template<class COMPONENT>
            [[nodiscard]] inline ECSComponentPool<COMPONENT>& getComponentPool(uint32_t typeID) const noexcept { 
                return *static_cast<ECSComponentPool<COMPONENT>*>(m_ComponentPools[typeID - 1].get()); 
            }
            template<class COMPONENT>
            [[nodiscard]] inline ECSComponentPool<COMPONENT>& getComponentPool(uint32_t typeID) noexcept { 
                return *static_cast<ECSComponentPool<COMPONENT>*>(m_ComponentPools[typeID - 1].get()); 
            }

            template<class COMPONENT>
            [[nodiscard]] inline const ECSComponentPool<COMPONENT>& getComponentPool() const noexcept { return getComponentPool<COMPONENT>(COMPONENT::TYPE_ID); }
            template<class COMPONENT>
            [[nodiscard]] inline ECSComponentPool<COMPONENT>& getComponentPool() noexcept { return getComponentPool<COMPONENT>(COMPONENT::TYPE_ID); }

            template<class COMPONENT>
            uint32_t registerComponent() noexcept {
                if (COMPONENT::TYPE_ID == 0) {
                    COMPONENT::TYPE_ID = ++m_RegisteredComponents;
                }
                auto threshold = std::max(COMPONENT::TYPE_ID, m_RegisteredComponents);
                if (m_ComponentPools.size() < threshold) {
                    m_ComponentPools.resize(threshold);
                }
                m_ComponentPools[COMPONENT::TYPE_ID - 1].reset(NEW ECSComponentPool<COMPONENT>{m_SceneOptions});
                return COMPONENT::TYPE_ID - 1;
            }

            template<class SYSTEM, class ARGS_TUPLE, class ... COMPONENTS>
            inline SYSTEM* registerSystem(ARGS_TUPLE&& argsTuple = ARGS_TUPLE{}) {
                return m_SystemPool.registerSystem<SYSTEM, ARGS_TUPLE, COMPONENTS...>(*this, std::forward<ARGS_TUPLE>(argsTuple));
            }
            template<class SYSTEM, class ARGS_TUPLE, class ... COMPONENTS>
            inline SYSTEM* registerSystemOrdered(uint32_t order, ARGS_TUPLE&& argsTuple = ARGS_TUPLE{}) {
                return m_SystemPool.registerSystemOrdered<SYSTEM, ARGS_TUPLE, COMPONENTS...>(order, *this, std::forward<ARGS_TUPLE>(argsTuple));
            }


            template<class COMPONENT>
            void onResize(uint32_t width, uint32_t height) noexcept {
                auto& componentPool = getComponentPool<COMPONENT>();
                for (auto& camera : componentPool) {
                    camera.comp.resize(width, height);
                }
            }

            void removeEntity(Entity);
            inline void update(const float dt, Scene& scene) noexcept {
                m_SystemPool.update(dt, scene);
            }
            inline void onSceneEntered(Scene& scene) noexcept {
                m_SystemPool.onSceneEntered(scene);
            }
            inline void onSceneLeft(Scene& scene) noexcept {
                m_SystemPool.onSceneLeft(scene);
            }

            //add newly created entities to the scene with their components as defined in their m_Systems, etc
            void preUpdate(Scene&, const float dt);
            //destroy flagged entities & their components, if any
            void postUpdate(Scene&, const float dt);

            template<class SYSTEM> [[nodiscard]] inline const SYSTEM& getSystem() const noexcept { return static_cast<const SYSTEM&>(m_SystemPool[SYSTEM::TYPE_ID - 1]); }
            template<class SYSTEM> [[nodiscard]] inline SYSTEM& getSystem() noexcept { return static_cast<SYSTEM&>(m_SystemPool[SYSTEM::TYPE_ID - 1]); }

            template<class COMPONENT, class ... ARGS> bool addComponent(Entity entity, ARGS&&... args) noexcept {
                COMPONENT* addedComponent = nullptr;
                bool result = false;
                {
                    std::lock_guard lock{ m_MutexRecursive };
                    addedComponent = getComponentPool<COMPONENT>().addComponent(entity, std::forward<ARGS>(args)...);
                    if (addedComponent) {
                        m_SystemPool.onComponentAddedToEntity(COMPONENT::TYPE_ID, addedComponent, entity);
                        result = true;
                    }
                }
                return result;
            }
            template<class COMPONENT> bool removeComponent(Entity entity) noexcept {
                bool didRemove  = false;
                {
                    std::lock_guard lock{ m_MutexRecursive };
                    didRemove = getComponentPool<COMPONENT>().removeComponent(entity);
                    if (didRemove) {
                        m_SystemPool.onComponentRemovedFromEntity(COMPONENT::TYPE_ID, entity);
                    }
                }
                return didRemove;
            }
            //gets the component. this is NOT thread safe
            template<class COMPONENT> [[nodiscard]] Engine::view_ptr<COMPONENT> getComponent(Entity entity) noexcept {
                return internal_get_component<COMPONENT>(entity);
            }
            //gets the components. this is NOT thread safe
            template<class ... TYPES> [[nodiscard]] std::tuple<Engine::view_ptr<TYPES>...> getComponents(Entity entity) noexcept { 
                return std::make_tuple(internal_get_component<TYPES>(entity)...);
            }
            //gets the component in a thread safe manner
            template<class COMPONENT> [[nodiscard]] Engine::view_ptr<COMPONENT> getComponentMt(Entity entity) noexcept {
                std::lock_guard lock{ m_MutexRecursive };
                return internal_get_component<COMPONENT>(entity);
            }
            //gets the components in a thread safe manner
            template<class ... TYPES> [[nodiscard]] std::tuple<Engine::view_ptr<TYPES>...> getComponentsMt(Entity entity) noexcept {
                std::lock_guard lock{ m_MutexRecursive };
                return std::make_tuple(internal_get_component<TYPES>(entity)...);
            }
    };
};

#endif
