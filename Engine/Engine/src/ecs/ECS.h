#ifndef ENGINE_ECS_H_INCLUDE_GUARD
#define ENGINE_ECS_H_INCLUDE_GUARD

#include <ecs/ECSRegistry.h>
#include <ecs/ECSEntityPool.h>
#include <ecs/ECSComponentPool.h>
#include <ecs/ECSSystem.h>
#include <ecs/Entity.h>
#include <memory>

namespace Engine {
    namespace epriv {
        class ECS{
            private:
                ECSEntityPool                                            entityPool;
                std::vector<std::unique_ptr<SparseSet<Entity>>>          componentPools;
                std::vector<ECSSystem<Entity>*>                          systems;

                //builds a component pool and system for the component type if it is not built already.
                template<typename TComponent> void buildPool(uint type_slot) {
                    using CSystemType = ECSSystem<Entity, TComponent>;
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (componentPools[type_slot]) return;
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1,nullptr);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<CPoolType>();
                    }
                    if (!systems[type_slot]) {
                        ECSSystemCI _ci;
                        systems[type_slot] = new CSystemType(_ci,*this);
                    }
                }
            public:
                ECS() = default;
                ~ECS() {
                    SAFE_DELETE_VECTOR(systems);
                }
                ECS(const ECS&) = delete;                      // non construction-copyable
                ECS& operator=(const ECS&) = delete;           // non copyable
                ECS(ECS&& other) noexcept = delete;            // non construction-moveable
                ECS& operator=(ECS&& other) noexcept = delete; // non moveable

                void update(const float& dt) {
                    for (uint i = 0; i < systems.size(); ++i) {
                        auto& system = *systems[i];
                        system.update(dt);
                    }
                }

                template<typename TComponent> ECSComponentPool<Entity, TComponent>& getPool() {
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    return *(CPoolType*)componentPools[type_slot].get();
                }
                template<typename TComponent> void assignSystem(ECSSystemCI& _systemCI) {
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    using CSystemType = ECSSystem<Entity, TComponent>;
                    if (type_slot >= componentPools.size()) {
                        componentPools.resize(type_slot + 1);
                    }
                    if (!componentPools[type_slot]) {
                        componentPools[type_slot] = std::make_unique<CPoolType>();
                    }
                    if (type_slot >= systems.size()) {
                        systems.resize(type_slot + 1);
                    }
                    if (systems[type_slot]) {
                        SAFE_DELETE(systems[type_slot]);
                    }
                    systems[type_slot] = new CSystemType(_systemCI,*this);
                }

                //we may or may not need these...
                Entity& addEntity(Scene& _scene) { return entityPool.addEntity(_scene); }
                void removeEntity(Entity& _entity) { entityPool.removeEntity(_entity); }
                EntityPOD* getEntity(Entity& _entity) { return entityPool.getEntity(_entity); }

                template<typename TComponent, typename... ARGS> TComponent* addComponent(const Entity& _entity, ARGS&&... _args) {
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.addComponent(_entity, std::forward<ARGS>(_args)...);
                }
                template<typename TComponent> bool removeComponent(const Entity& _entity) {
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.removeComponent(_entity);
                }
                template<typename TComponent> TComponent* getComponent(const Entity& _entity) {
                    using CPoolType = ECSComponentPool<Entity, TComponent>;
                    uint type_slot = ECSRegistry::type_slot<TComponent>();
                    buildPool<TComponent>(type_slot);
                    auto& cPool = *(CPoolType*)componentPools[type_slot].get();
                    return cPool.getComponent(_entity);
                }
        };
    };
};

#endif
