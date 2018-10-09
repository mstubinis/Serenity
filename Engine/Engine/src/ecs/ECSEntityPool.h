#pragma once
#ifndef ENGINE_ECS_ENTITY_POOL_H
#define ENGINE_ECS_ENTITY_POOL_H

namespace Engine {
    namespace epriv {
        template<typename TEntity> class ECSEntityPool final {
            private:
                uint lastIndex;
                std::vector<TEntity> pool;
            public:
                ECSEntityPool() :lastIndex(0) {}
                ~ECSEntityPool() { lastIndex = 0; pool.clear(); }
                TEntity* getEntity(uint _entityID) {
                    if (_entityID == 0) return nullptr;
                    return &pool[_entityID - 1];
                }
                TEntity& createEntity(Scene& _scene) {
                    pool.emplace_back(lastIndex + 1, _scene);
                    ++lastIndex;
                    return pool[pool.size() - 1];
                }
                void addEntity(const TEntity& _entity) {
                    pool.push_back(std::move(_entity));
                    ++lastIndex;
                }
                void removeEntity(uint _entityID) {
                    uint indexToRemove = _entityID - 1;
                    if (indexToRemove != lastIndex) {
                        std::swap(pool[indexToRemove], pool[lastIndex]);
                    }
                    TEntity& e = pool[pool.size()];
                    pool.pop_back();
                    --lastIndex;
                }
                void removeEntity(TEntity& _entity) { removeEntity(_entity.ID); }
                void moveEntity(ECSEntityPool<TEntity>& other, uint _entityID) {
                    TEntity& e = pool[_entityID - 1];
                    other.addEntity(e);
                    removeEntity(e);
                }
                void moveEntity(ECSEntityPool<TEntity>& other, TEntity& _entity) { moveEntity(other, _entity.ID); }
        };
    };
};

#endif