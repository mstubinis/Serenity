#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

struct SceneOptions;
namespace Engine::priv {
    struct InternalScenePublicInterface;
}

struct Entity;
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSSystem.h>
#include <ecs/ECSComponentPool.h>
#include <core/engine/scene/SceneOptions.h>

namespace Engine::priv {
    class ECS final {
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            Engine::type_registry                       m_Registry;
            ECSEntityPool                               m_EntityPool;
            std::vector<Entity>                         m_JustAddedEntities;
            std::vector<Entity>                         m_DestroyedEntities;
            std::vector<Engine::priv::sparse_set_base*> m_ComponentPools;
            std::vector<ECSSystemBase*>                 m_Systems;
            std::mutex                                  m_Mutex;
            SceneOptions                                m_SceneOptions;
        public:
            ECS() = default;
            ECS(const ECS&)                      = delete;
            ECS& operator=(const ECS&)           = delete;
            ECS(ECS&& other) noexcept            = delete;
            ECS& operator=(ECS&& other) noexcept = delete;
            ~ECS();

            void init(const SceneOptions& options);

            void clearAllEntities() noexcept;

            inline const ECSEntityPool& getEntityPool() const noexcept {
                return m_EntityPool;
            }
            Entity createEntity(Scene& scene);

            template<typename COMPONENT>
            void onResize(uint32_t width, uint32_t height) {
                using CPoolType  = ECSComponentPool<COMPONENT>;
                auto type_slot   = m_Registry.type_slot_fast<COMPONENT>();
                auto& components = (*(CPoolType*)m_ComponentPools[type_slot]).data();
                for (auto& camera : components) {
                    camera.resize(width, height);
                }
            }

            void removeEntity(Entity entity);
            void update(const float dt, Scene& scene);
            void onComponentAddedToEntity(void* component, Entity entity, uint32_t type_slot);
            void onComponentRemovedFromEntity(Entity entity, uint32_t type_slot);
            void onSceneEntered(Scene& scene);
            void onSceneLeft(Scene& scene);

            //add newly created entities to the scene with their components as defined in their m_Systems, etc
            void preUpdate(Scene& scene, const float dt);
            //destroy flagged entities & their components, if any
            void postUpdate(Scene& scene, const float dt);

            template<class COMPONENT> inline CONSTEXPR ECSComponentPool<COMPONENT>& getPool() const {
                using CPOOL = ECSComponentPool<COMPONENT>;
                auto type_slot  = m_Registry.type_slot_fast<COMPONENT>();
                return *(CPOOL*)m_ComponentPools[type_slot];
            }

            template<class COMPONENT> inline CONSTEXPR ECSSystem<COMPONENT>& getSystem() const {
                auto type_slot = m_Registry.type_slot_fast<COMPONENT>();
                return *(ECSSystem<COMPONENT>*)m_Systems[type_slot];
            }


            template<class COMPONENT>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot = m_Registry.type_slot<COMPONENT>();
                using CPOOL    = ECSComponentPool<COMPONENT>;
                using SYSTEM   = ECSSystem<COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPOOL{ m_SceneOptions };
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW SYSTEM( m_SceneOptions, systemCI, getPool<COMPONENT>() );
            }
            template<class COMPONENT, typename SYSTEM>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot = m_Registry.type_slot<COMPONENT>();
                using CPOOL    = ECSComponentPool<COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPOOL{ m_SceneOptions };
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW SYSTEM( m_SceneOptions, systemCI, getPool<COMPONENT>() );
            }

            template<class T, typename... ARGS> bool addComponent(Entity entity, ARGS&&... args) noexcept {
                auto type_slot = m_Registry.type_slot_fast<T>();
                auto& cPool    = *(ECSComponentPool<T>*)m_ComponentPools[type_slot];
                T* res         = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                    return true;
                }
                return false;
            }
            template<class T> bool removeComponent(Entity entity) noexcept {
                auto type_slot = m_Registry.type_slot_fast<T>();
                auto& cPool    = *(ECSComponentPool<T>*)m_ComponentPools[type_slot];
                bool ret_val   = false;
                {
                    std::lock_guard lock(m_Mutex);
                    ret_val = cPool.removeComponent(entity);
                    if (ret_val) {
                        onComponentRemovedFromEntity(entity, type_slot);
                    }
                }
                return ret_val;
            }

            template<class T> inline CONSTEXPR Engine::view_ptr<T> getComponent(Entity entity) const noexcept {
                using CPOOL = ECSComponentPool<T>;
                return ((CPOOL*)m_ComponentPools[m_Registry.type_slot_fast<T>()])->getComponent(entity);
            }

            template<class... TYPES> inline CONSTEXPR std::tuple<Engine::view_ptr<TYPES>...> getComponents(Entity entity) const noexcept {
                return std::make_tuple(getComponent<TYPES>(entity)...);
            }
    };
};

#endif
