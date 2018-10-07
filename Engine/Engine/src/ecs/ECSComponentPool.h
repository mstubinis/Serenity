#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>

namespace Engine {
    namespace epriv {
        template <typename...>      class ECSComponentPool;
        template <typename TEntity> class ECSComponentPool<TEntity>{
            protected:
                uint                              amount;   //number of components created
              //std::vector<TEntity>              sparse; //maps entity ID to component Index in dense
                std::vector<uint>                 sparse; //maps entity ID to component Index in dense
            public:
                ECSComponentPool() : amount(0) {}
                virtual ~ECSComponentPool() { amount = 0; if (sparse.size() > 0) sparse.clear(); }
        };


        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public ECSComponentPool<TEntity>{
            using super = ECSComponentPool<TEntity>;
            public:
                std::vector<TComponent>           dense;  //actual component pool

                ECSComponentPool() = default;
                ECSComponentPool(const ECSComponentPool& other) = default;
                ECSComponentPool& operator=(const ECSComponentPool& other) = default;
                ~ECSComponentPool() { if(dense.size() > 0) dense.clear(); }
                template<typename... ARGS> TComponent* addComponent(TEntity& _entity, ARGS&&... _args) {
                    uint sparseID = _entity.ID - 1;
                    if (sparseID >= super::sparse.size())
                        super::sparse.resize(sparseID + 1,0);
                    if (super::sparse[sparseID] != 0)
                        return nullptr;
                    dense.emplace_back(_entity, std::forward<ARGS>(_args)...);
                    ++super::amount;
                    super::sparse[sparseID] = super::amount;
                    return &dense[super::amount - 1];
                }
                bool removeComponent(TEntity& _entity) {
                    uint sparseID = _entity.ID - 1;
                    if (super::sparse[sparseID] == 0)
                        return false;
                    uint removedCID = super::sparse[sparseID];
                    if (dense.size() > 1){
                        std::swap(dense[removedCID], dense[super::amount]);
                        super::sparse[super::amount] = removedCID;
                    }
                    --super::amount;
                    dense.pop_back();
                    return true;
                }
                TComponent* getComponent(TEntity& _entity) {
                    uint sparseID = _entity.ID - 1;
                    if (super::sparse.size() == 0)
                        return nullptr;
                    if (super::sparse[sparseID] == 0)
                        return nullptr;
                    return &(dense[super::sparse[sparseID]]);
                }
        };






    };
};
#endif