#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

//#include "Entity.h"
#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>

namespace Engine {
    namespace epriv {
        template<typename...> class ECSComponentPool;
        template <typename TEntity> class ECSComponentPool<TEntity>{
            protected:
                uint                              amount;   //number of components created
              //std::vector<TEntity>              sparse; //maps entity ID to component Index in dense
                std::vector<uint>                 sparse; //maps entity ID to component Index in dense
            public:
                ECSComponentPool() : amount(0) {}
                ~ECSComponentPool() { amount = 0; sparse.clear(); }
        };


        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public ECSComponentPool<TEntity>{
            using super = ECSComponentPool<TEntity>;
            private:
                std::vector<TComponent>           dense;  //actual component pool
            public:
                ECSComponentPool(){
                }
                ~ECSComponentPool() {
                    dense.clear();
                }
                TComponent* addComponent(uint _entityID) {
                    uint sparseID = _entityID - 1;
                    if (!(sparseID < super::sparse.size()))
                        super::sparse.resize(sparseID + 1,0);
                    if (super::sparse[sparseID] != 0)
                        return nullptr;
                    dense.push_back(TComponent());
                    //dense.emplace_back(TComponent()); use this instead?
                    ++super::amount;
                    super::sparse[sparseID] = super::amount;
                    return &dense[super::amount];
                }
                bool removeComponent(uint _entityID) {
                    uint sparseID = _entityID - 1;
                    if (super::sparse[sparseID] == 0)
                        return false;
                    uint removedCID = super::sparse[sparseID];
                    std::swap(dense[removedCID], dense[super::amount]);
                    --super::amount;
                    super::sparse[super::amount] = removedCID;
                    dense.pop_back();
                    return true;
                }
                TComponent* getComponent(uint _entityID) {
                    uint sparseID = _entityID - 1;
                    if (super::sparse[sparseID] == 0)
                        return nullptr;
                    return &(dense[super::sparse[sparseID]]);
                }
        };






    };
};
#endif