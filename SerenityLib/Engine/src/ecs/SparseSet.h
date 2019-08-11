#pragma once
#ifndef ENGINE_ECS_SPARSE_SET_H
#define ENGINE_ECS_SPARSE_SET_H

#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <core/engine/utils/Utils.h>


namespace Engine {
namespace epriv {

    template <typename...>      class SparseSet;
    template <typename TID>     class SparseSet<TID> {
        protected:
            uint                              _maxLast;
            std::vector<uint>                 _sparse; //maps entity ID to component Index in dense
        public:
            std::vector<uint>& sparse() { return _sparse; }

            SparseSet() { 
                _maxLast = 0; 
            }
            virtual ~SparseSet() { 
                _maxLast = 0;
                _sparse.clear(); 
            }
            virtual bool _remove(const uint& _IDObject) { 
                return false; 
            }
    };
    template <typename TID, typename T> class SparseSet<TID, T> : public SparseSet<TID> {
        using super = SparseSet<TID>;
        private:
            std::vector<T> _dense;
        public:
			std::vector<T>& pool() { return _dense; }

            SparseSet()                                           = default;
            SparseSet(const SparseSet& other) noexcept            = delete;
            SparseSet& operator=(const SparseSet& other) noexcept = delete;
            SparseSet(SparseSet&& other) noexcept                 = delete;
            SparseSet& operator=(SparseSet&& other) noexcept      = delete;
            virtual ~SparseSet()                                  = default;

            template<typename... ARGS> inline T* _add(const uint& entityID, ARGS&&... args) {
                const uint& sparseIndex = entityID - 1;
                if (sparseIndex >= super::_sparse.size())
                    super::_sparse.resize(sparseIndex + 1, 0);
                if (super::_sparse[sparseIndex] != 0)
                    return nullptr;
                _dense.emplace_back(std::forward<ARGS>(args)...);
                super::_sparse[sparseIndex] = _dense.size();
                super::_maxLast = sparseIndex;
                return &_dense[_dense.size() - 1];
            }
            //TODO: this entire function needs a serious look at
            inline bool _remove(const uint& entityID) {
                //TODO: find a way to optimize the search for the maxLast entity...
                const uint removedEntityIndex = entityID - 1;
                if (removedEntityIndex >= super::_sparse.size()) { //needed for scene.removeEntity(), as it iterates over all systems and some might not have the entity in them
                    return false;
                }
                const uint removedComponentID = super::_sparse[removedEntityIndex];
                if (removedComponentID == 0) {
                    return false;
                }
                super::_sparse[removedEntityIndex] = 0;
                //if (removedEntityIndex >= super::_maxLast) { 
                    uint currMax = 0;
                    super::_maxLast = 0;
                    for (uint i = super::_sparse.size(); i-- > 0;) { 
                        if (super::_sparse[i] > 0) { 
                            super::_maxLast = i; 
                            currMax = super::_sparse[i];
                            break; 
                        } 
                    } 
                //}
                if (_dense.size() > 1) {
                    using std::swap;
                    const auto firstIndex = removedComponentID - 1;
                    const auto secondIndex = _dense.size() - 1;
                    if (firstIndex != secondIndex) {
                        if (firstIndex < secondIndex) {
                            swap(_dense[firstIndex], _dense[secondIndex]);
                            super::_sparse[super::_maxLast] = removedComponentID;
                        }else{
                            const uint last = currMax - 1;
                            super::_sparse[super::_maxLast] = last;
                        }
                    }
                    
                }
                _dense.pop_back();
                return true;
            }
            inline T* _get(const uint& entityID) {
                const uint sparseIndex = entityID - 1;
                const uint sparseSize  = super::_sparse.size();
                if (sparseSize == 0 || sparseIndex >= sparseSize || super::_sparse[sparseIndex] == 0 )
                    return nullptr;
                const uint retIndex = super::_sparse[sparseIndex] - 1;
                if(retIndex >= _dense.size())
                    return nullptr;
                return &_dense[retIndex];
            }
    };
};
};

#endif