#pragma once
#ifndef ENGINE_ECS_H
#define ENGINE_ECS_H

struct SceneOptions;
namespace Engine::priv {
    struct InternalScenePublicInterface;
}

//#include <core/engine/utils/Utils.h>
#include <ecs/ECSRegistry.h>
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSSystem.h>
#include <mutex>

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
            /*
            //builds a component pool and system for the component type if it is not built already.
            template<typename COMPONENT> 
            void buildPool(std::uint32_t type_slot) {
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (m_ComponentPools[type_slot]) {
                    return;
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW ECSComponentPool<ENTITY, COMPONENT>();
                }
                if (!m_Systems[type_slot]) {
                    ECSSystemCI _ci;
                    m_Systems[type_slot] = NEW ECSSystem<ENTITY, COMPONENT>(_ci, *this);
                }
            }
            */
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


            //"event handlers"
            template<typename COMPONENT>
            void onResize(std::uint32_t width, std::uint32_t height) {
                using CPoolType  = ECSComponentPool<ENTITY, COMPONENT>;
                auto type_slot   = ECSRegistry::type_slot_fast<COMPONENT>();
                auto& components = (*static_cast<CPoolType*>(m_ComponentPools[type_slot])).data();
                for (auto& camera : components) {
                    camera.resize(width, height);
                }
            }

            //update all component systems
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
                for (auto& entity : m_DestroyedEntities) {
                    EntityDataRequest request(entity);
                    auto id = request.ID;
                    m_EntityPool.destroyFlaggedEntity(id);
                    for (size_t i = 0; i < m_ComponentPools.size(); ++i) {
                        m_ComponentPools[i]->remove(id);
                        m_Systems[i]->onComponentRemovedFromEntity(entity);
                    }
                    m_DestroyedEntities.clear();
                }
                for (auto& component_pool : m_ComponentPools) {
                    component_pool->reserve(150);
                }
            }

            template<class COMPONENT>
            ECSComponentPool<ENTITY, COMPONENT>& getPool() const {
                using CPoolType = ECSComponentPool<ENTITY, COMPONENT>;
                auto type_slot  = ECSRegistry::type_slot_fast<COMPONENT>();
                return *static_cast<CPoolType*>(m_ComponentPools[type_slot]);
            }

            template<class COMPONENT>
            inline ECSSystem<ENTITY, COMPONENT>& getSystem() const {
                auto type_slot = ECSRegistry::type_slot_fast<COMPONENT>();
                return *static_cast<ECSSystem<ENTITY, COMPONENT>*>(m_Systems[type_slot]);
            }


            template<class COMPONENT>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot    = ECSRegistry::type_slot<COMPONENT>();
                using CPoolType   = ECSComponentPool<ENTITY, COMPONENT>;
                using CSystemType = ECSSystem<ENTITY, COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPoolType();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW CSystemType(systemCI, *this);
            }
            template<class COMPONENT, typename TSystem>
            void assignSystem(const ECSSystemCI& systemCI/*, unsigned int sortValue*/) {
                auto type_slot  = ECSRegistry::type_slot<COMPONENT>();
                using CPoolType = ECSComponentPool<ENTITY, COMPONENT>;
                if (type_slot >= m_ComponentPools.size()) {
                    m_ComponentPools.resize(type_slot + 1, nullptr);
                }
                if (!m_ComponentPools[type_slot]) {
                    m_ComponentPools[type_slot] = NEW CPoolType();
                }
                if (type_slot >= m_Systems.size()) {
                    m_Systems.resize(type_slot + 1, nullptr);
                }
                if (m_Systems[type_slot]) {
                    SAFE_DELETE(m_Systems[type_slot]);
                }
                m_Systems[type_slot] = NEW TSystem(systemCI, *this);
            }

            ENTITY createEntity(Scene& scene) {
                ENTITY res = m_EntityPool.addEntity(scene);
                m_JustAddedEntities.push_back(res);
                return std::move(res);
            }
            void removeEntity(ENTITY entity) {
                m_DestroyedEntities.push_back(entity);
            }
            /*
            Entity getEntity(unsigned int entityID) const {
                return m_EntityPool.getEntity(entityID); 
            }
            */
            template<class T, typename... ARGS> void addComponent(ENTITY entity, ARGS&&... args) {
                auto type_slot = ECSRegistry::type_slot_fast<T>();
                auto& cPool    = *static_cast<ECSComponentPool<ENTITY, T>*>(m_ComponentPools[type_slot]);
                T* res         = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
            }
            template<class T, typename... ARGS> void addComponent(const EntityDataRequest& request, ENTITY entity, ARGS&&... args) {
                auto type_slot = ECSRegistry::type_slot_fast<T>();
                auto& cPool    = *static_cast<ECSComponentPool<ENTITY, T>*>(m_ComponentPools[type_slot]);
                T* res         = nullptr;
                {
                    std::lock_guard lock(m_Mutex);
                    res = cPool.addComponent(request, entity, std::forward<ARGS>(args)...);
                }
                if (res) {
                    onComponentAddedToEntity(res, entity, type_slot);
                }
            }
            template<class T> bool removeComponent(ENTITY entity) {
                auto type_slot = ECSRegistry::type_slot_fast<T>();
                auto& cPool    = *static_cast<ECSComponentPool<ENTITY, T>*>(m_ComponentPools[type_slot]);
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
#pragma region 1 component get
            template<class T> inline T* getComponent(ENTITY& entity) const {
                using CPoolType = ECSComponentPool<ENTITY, T>;
                return static_cast<CPoolType*>(m_ComponentPools[ECSRegistry::type_slot_fast<T>()])->getComponent(entity);
            }
            template<class T> inline T* getComponent(const EntityDataRequest& dataRequest) const {
                using CPoolType = ECSComponentPool<ENTITY, T>;
                return static_cast<CPoolType*>(m_ComponentPools[ECSRegistry::type_slot_fast<T>()])->getComponent(dataRequest);
            }
#pragma endregion

#pragma region variadic component get
            template<class... Types> inline std::tuple<Types*...> getComponents(ENTITY& entity) const {
                EntityDataRequest dataRequest(entity);
                return ECS::getComponents<Types...>(dataRequest);
            }
            template<class... Types> inline std::tuple<Types*...> getComponents(const EntityDataRequest& dataRequest) const {
                return std::make_tuple(getComponent<Types>(dataRequest)... );
            }
#pragma endregion
    };
};

#endif
