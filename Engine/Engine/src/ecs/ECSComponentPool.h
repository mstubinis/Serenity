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
                uint                              _amount;   //number of components created
              //std::vector<TEntity>              _sparse; //maps entity ID to component Index in dense
                std::vector<uint>                 _sparse; //maps entity ID to component Index in dense
            public:
                std::vector<uint>& sparse() { return _sparse; }

                ECSComponentPool() { _amount = 0; }
                virtual ~ECSComponentPool() { _amount = 0; _sparse.clear(); }
        };
        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public ECSComponentPool<TEntity>{
            using super = ECSComponentPool<TEntity>;
            private:
                std::vector<TComponent>           _dense;  //actual component pool
            public:
                std::vector<TComponent>& dense() { return _dense; }
                TComponent& component(uint _EntityID) { return _dense[super::_sparse[_EntityID - 1]]; }
                TComponent& component(TEntity& _Entity) { return component(_Entity.ID); }

                ECSComponentPool() = default;
                ECSComponentPool(const ECSComponentPool& other) = default;
                ECSComponentPool& operator=(const ECSComponentPool& other) = default;
                ~ECSComponentPool() { }
                template<typename... ARGS> TComponent* addComponent(const TEntity& _entity, ARGS&&... _args) {
                    uint sparseID = _entity.ID - 1;
                    if (sparseID >= super::_sparse.size())
                        super::_sparse.resize(sparseID + 1,0);
                    if (super::_sparse[sparseID] != 0)
                        return nullptr;
                    _dense.emplace_back(const_cast<TEntity&>(_entity), std::forward<ARGS>(_args)...);
                    ++super::_amount;
                    super::_sparse[sparseID] = super::_amount;
                    return &_dense[super::_amount - 1];
                }
                bool removeComponent(const TEntity& _entity) {
                    uint sparseID = _entity.ID - 1;
                    if (super::_sparse[sparseID] == 0)
                        return false;
                    uint removedCID = super::_sparse[sparseID];
                    if (_dense.size() > 1){
                        std::swap(_dense[removedCID], _dense[super::_amount]);
                        super::_sparse[super::_amount] = removedCID;
                    }
                    --super::_amount;
                    _dense.pop_back();
                    return true;
                }
                TComponent* getComponent(const TEntity& _entity) {
                    uint sparseID = _entity.ID - 1;
                    if (super::_sparse.size() == 0)
                        return nullptr;
                    if (super::_sparse[sparseID] == 0)
                        return nullptr;
                    return &(_dense[super::_sparse[sparseID]]);
                }
        };
    };
};
#endif