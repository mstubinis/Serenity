#pragma once
#ifndef ENGINE_ECS_COMPONENT_POOL_H
#define ENGINE_ECS_COMPONENT_POOL_H

#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <vector>
#include <iostream>

namespace Engine {
    namespace epriv {
        template <typename...>      class ECSComponentPool;
        template <typename TEntity> class ECSComponentPool<TEntity>{
            protected:
                uint                              _maxLast;
                std::vector<uint>                 _sparse; //maps entity ID to component Index in dense
            public:
                std::vector<uint>& sparse() { return _sparse; }

                ECSComponentPool() { _maxLast = 0; }
                virtual ~ECSComponentPool() { _maxLast = 0; _sparse.clear(); }
        };
        template <typename TEntity,typename TComponent> class ECSComponentPool<TEntity,TComponent> : public ECSComponentPool<TEntity>{
            using super = ECSComponentPool<TEntity>;
            private:
                std::vector<TComponent>                      _dense;  //actual component pool
            public:
                std::vector<TComponent>& dense() { return _dense; }

                ECSComponentPool() = default;
                ECSComponentPool(const ECSComponentPool& other) = default;
                ECSComponentPool& operator=(const ECSComponentPool& other) = default;
                ECSComponentPool(ECSComponentPool&& other) noexcept = default;
                ECSComponentPool& operator=(ECSComponentPool&& other) noexcept = default;

                ~ECSComponentPool() = default;

                template<typename... ARGS> TComponent* addComponent(const TEntity& _entity, ARGS&&... _args) {
                    uint sparseIndex = _entity.ID - 1;
                    if (sparseIndex >= super::_sparse.size())
                        super::_sparse.resize(sparseIndex + 1,0);
                    if (super::_sparse[sparseIndex] != 0)
                        return nullptr;
                    _dense.emplace_back(const_cast<TEntity&>(_entity), std::forward<ARGS>(_args)...);
                    super::_sparse[sparseIndex] = _dense.size();
                    super::_maxLast = sparseIndex;
                    return &_dense[_dense.size() - 1];
                }
                bool removeComponent(const TEntity& _entity) {
                    //TODO: find a way to optimize the search for the maxLast entity...
                    uint removedEntityIndex = _entity.ID - 1;
                    uint removedComponentID = super::_sparse[removedEntityIndex];
                    super::_sparse[removedEntityIndex] = 0;
                    if (removedComponentID == 0)
                        return false;
                    if (removedEntityIndex >= super::_maxLast) { super::_maxLast = 0; for (uint i = super::_sparse.size(); i-- > 0;) { if (super::_sparse[i] > 0) { super::_maxLast = i; break; } } }
                    if (_dense.size() > 1){
                        std::swap(_dense[removedComponentID - 1], _dense[_dense.size() - 1]);
                        super::_sparse[super::_maxLast] = removedComponentID;
                    }
                    _dense.pop_back();
                    return true;
                }
                TComponent* getComponent(const TEntity& _entity) {
                    uint sparseIndex = _entity.ID - 1;
                    if (super::_sparse.size() == 0)
                        return nullptr;
                    if (super::_sparse[sparseIndex] == 0)
                        return nullptr;
                    return &(_dense[super::_sparse[sparseIndex]]);
                }
        };
    };
};
#endif