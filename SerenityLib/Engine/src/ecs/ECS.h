#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

struct SceneOptions;
namespace Engine::priv {
    struct InternalScenePublicInterface;
}

#include <ecs/ECSRegistry.h>
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSSystem.h>

namespace Engine::priv {
    template<typename ENTITY> 
    class ECS final {
        friend struct Engine::priv::InternalScenePublicInterface;
        private:
            ECSEntityPool<ENTITY>                       m_EntityPool;
            std::vector<ENTITY>                         m_JustAddedEntities;
            std::vector<ENTITY>                         m_DestroyedEntities;
            std::vector<Engine::priv::sparse_set_base*> m_ComponentPools;
            std::vector<ECSSystem<ENTITY>*>             m_Systems;
            std::mutex                                  m_Mutex;

        public:
            ECS(/*const SceneOptions& options*/) {
            }
            ~ECS() {
                SAFE_DELETE_VECTOR(m_ComponentPools);
                SAFE_DELETE_VECTOR(m_Systems);
            }
            ECS(const ECS&)                      = delete;
            ECS& operator=(const ECS&)           = delete;
            ECS(ECS&& other) noexcept            = delete;
            ECS& operator=(ECS&& other) noexcept = delete;

            inline CONSTEXPR const ECSEntityPool<ENTITY>& getEntityPool() const noexcept { return m_EntityPool; }


            //"event handlers"
            template<typename COMPONENT>
            void onResize(std::uint32_t width, std::uint32_t height) {
                using CPoolType  = ECSComponentPool<ENTITY, COMPONENT>;
                auto type_slot   = ECSRegistry::type_slot_fast<COMPONENT>();
                auto& components = (*(CPoolType*)m_ComponentPools[type_slot]).data();
                for (auto& camera : components) {
                    camera.resize(width, height);
                }
            }


            void removeEntity(ENTITY entity) {
                m_DestroyedEntities.emplace_back(entity);
            }
            void update(const float dt, Scene& scene) {
                for (size_t i = 0; i < m_Systems.size(); ++i) { 
                    m_Systems[i]->onUpdate(dt, scene);
                }
            }
            void onComponentAddedToEntity(void* component, ENTITY entity, std::uint32_t type_slot) {
                m_Systems[type_slot]->onComponentAddedToEntity(component, entity);
            }
            void onComponentRemovedFromEntity(ENTITY entity, std::uint32_t type_slot) {
                m_Systems[type_slot]->onComponentRemovedFromEntity(entity);
            }
            void onSceneEntered(Scene& scene) { 
                for (size_t i = 0; i < m_Systems.size(); ++i) { 
                    m_Systems[i]->onSceneEntered(scene);
                }
            }
            void onSceneLeft(Scene& scene) { 
                for (size_t i = 0; i < m_Systems.size(); ++i) { 
                    m_Systems[i]->onSceneLeft(scene);
                }
            }
            //add newly created entities to the scene with their components as defined in their m_Systems, etc
            void preUpdate(Scene& scene, const float dt) {
                if (m_JustAddedEntities.size() > 0) {
                    for (size_t i = 0; i < m_Systems.size(); ++i) {
                        for (size_t j = 0; j < m_JustAddedEntities.size(); ++j) {
                            m_Systems[i]->onEntityAddedToScene(m_JustAddedEntities[j], scene);
                        }
                    }
                    m_JustAddedEntities.clear();
                }
            }
            //destroy flagged entities & their components, if any
            void postUpdate(Scene& scene, const float dt) {
                if (m_DestroyedEntities.size() > 0) {
                    for (const auto entity : m_DestroyedEntities) {
                        auto id = entity.id();
                        m_EntityPool.destroyFlaggedEntity(id);
                        for (size_t i = 0; i < m_ComponentPools.size(); ++i) {
                            m_ComponentPools[i]->remove(id);
                            m_Systems[i]->onComponentRemovedFromEntity(entity);
                        }
                    }
                    m_DestroyedEntities.clear();
                    for (auto& component_pool : m_ComponentPools) {
                        component_pool->reserve(150);
                    }
                }
            }

            template<class COMPONENT> inline CONSTEXPR ECSComponentPool<ENTITY, COMPONENT>& getPool() const {
                using CPOOL = ECSComponentPool<ENTITY, COMPONENT>;
                auto type_slot  = ECSRegistry::type_slot_fast<COMPONENT>();
                return *(CPOOL*)m_ComponentPools[type_slot];
            }

            template<class COMPONENT> inline CONSTEXPR ECSSystem<ENTITY, COMPONENT>& getSystem() const {
                auto type_slot = ECSRegistry::type_slot_fast<COMPONENT>();
                return *(ECSSystem<ENTITY, COMPONENT>*)m_Systems[type_slot];
            }


            template<class COMPONENT>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot = ECSRegistry::type_slot<COMPONENT>();
                using CPOOL    = ECSComponentPool<ENTITY, COMPONENT>;
                using SYSTEM   = ECSSystem<ENTITY, COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPOOL();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW SYSTEM(systemCI, *this);
            }
            template<class COMPONENT, typename SYSTEM>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot = ECSRegistry::type_slot<COMPONENT>();
                using CPOOL    = ECSComponentPool<ENTITY, COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPOOL();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW SYSTEM(systemCI, *this);
            }

            ENTITY createEntity(Scene& scene) {
                ENTITY res = m_EntityPool.addEntity(scene);
                m_JustAddedEntities.push_back(res);
                return std::move(res);
            }
            template<class T, typename... ARGS> void addComponent(ENTITY entity, ARGS&&... args) noexcept {
                auto type_slot = ECSRegistry::type_slot_fast<T>();
                auto& cPool    = *(ECSComponentPool<ENTITY, T>*)m_ComponentPools[type_slot];
                T* res         = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
            }
            template<class T> bool removeComponent(ENTITY entity) noexcept {
                auto type_slot = ECSRegistry::type_slot_fast<T>();
                auto& cPool    = *(ECSComponentPool<ENTITY, T>*)m_ComponentPools[type_slot];
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

            template<class T> inline CONSTEXPR T* getComponent(ENTITY entity) const noexcept {
                using CPOOL = ECSComponentPool<ENTITY, T>;
                return ((CPOOL*)m_ComponentPools[ECSRegistry::type_slot_fast<T>()])->getComponent(entity);
            }

            template<class... TYPES> inline CONSTEXPR std::tuple<TYPES*...> getComponents(ENTITY entity) const noexcept {
                return std::make_tuple(getComponent<TYPES>(entity)...);
            }
    };
};

#endif
