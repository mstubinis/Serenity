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
            uint32_t                                        m_RegisteredComponents = 0;
            ECSEntityPool                                   m_EntityPool;
            ECSSystemPool                                   m_SystemPool;
            std::vector<Entity>                             m_JustAddedEntities;
            std::vector<Entity>                             m_DestroyedEntities;
            std::vector<std::unique_ptr<sparse_set_base>>   m_ComponentPools;
            std::mutex                                      m_Mutex;
            SceneOptions                                    m_SceneOptions;
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
                if (m_ComponentPools.size() < COMPONENT::TYPE_ID) {
                    m_ComponentPools.resize(COMPONENT::TYPE_ID);
                    m_ComponentPools[COMPONENT::TYPE_ID - 1].reset( NEW ECSComponentPool<COMPONENT>{m_SceneOptions} );
                }
                return COMPONENT::TYPE_ID - 1;
            }
            template<class COMPONENT>
            void onResize(uint32_t width, uint32_t height) noexcept {
                auto& componentPool = getComponentPool<COMPONENT>();
                for (auto& camera : componentPool) {
                    camera.resize(width, height);
                }
            }

            void removeEntity(Entity);
            void update(const float dt, Scene&);
            void onSceneEntered(Scene&);
            void onSceneLeft(Scene&);

            //add newly created entities to the scene with their components as defined in their m_Systems, etc
            void preUpdate(Scene&, const float dt);
            //destroy flagged entities & their components, if any
            void postUpdate(Scene&, const float dt);

            template<class COMPONENT> [[nodiscard]] inline const SystemBaseClass& getSystem() const noexcept { return m_SystemPool[COMPONENT::TYPE_ID - 1]; }
            template<class COMPONENT> [[nodiscard]] inline SystemBaseClass& getSystem() noexcept { return m_SystemPool[COMPONENT::TYPE_ID - 1]; }

            template<class SYSTEM, class ... COMPONENTS, class ... ARGS>
            inline SYSTEM* registerSystem(ARGS&&... args) { return m_SystemPool.registerSystem<SYSTEM, COMPONENTS..., ARGS...>(*this, std::forward<ARGS>(args)...); }

            template<class COMPONENT, class ... ARGS> bool addComponent(Entity entity, ARGS&&... args) noexcept {
                COMPONENT* addedComponent = nullptr;
                {
                    std::lock_guard lock{ m_Mutex };
                    addedComponent = getComponentPool<COMPONENT>().addComponent(entity, std::forward<ARGS>(args)...);
                }
                if (addedComponent) {
                    m_SystemPool.onComponentAddedToEntity(COMPONENT::TYPE_ID, addedComponent, entity);
                    return true;
                }
                return false;
            }
            template<class COMPONENT> bool removeComponent(Entity entity) noexcept {
                bool didRemove  = false;
                {
                    std::lock_guard lock{ m_Mutex };
                    didRemove = getComponentPool<COMPONENT>().removeComponent(entity);
                    if (didRemove) {
                        m_SystemPool.onComponentRemovedFromEntity(COMPONENT::TYPE_ID, entity);
                    }
                }
                return didRemove;
            }

            template<class COMPONENT> [[nodiscard]] inline constexpr Engine::view_ptr<COMPONENT> getComponent(Entity entity) const noexcept {
                return getComponentPool<COMPONENT>().getComponent(entity);
            }

            template<class ... TYPES> [[nodiscard]] inline constexpr std::tuple<Engine::view_ptr<TYPES>...> getComponents(Entity entity) const noexcept {
                return std::make_tuple( getComponent<TYPES>(entity)... );
            }
    };
};

#endif
