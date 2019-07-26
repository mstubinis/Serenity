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

            template<typename... ARGS> inline T* _add(const uint& IDObject, ARGS&&... args) {
                const uint& sparseIndex = IDObject - 1;
                if (sparseIndex >= super::_sparse.size())
                    super::_sparse.resize(sparseIndex + 1, 0);
                if (super::_sparse[sparseIndex] != 0)
                    return nullptr;
                _dense.emplace_back(std::forward<ARGS>(args)...);
                super::_sparse[sparseIndex] = _dense.size();
                super::_maxLast = sparseIndex;
                return &_dense[_dense.size() - 1];
            }
            inline bool _remove(const uint& IDObject) {
                //TODO: find a way to optimize the search for the maxLast entity...
                const uint& removedEntityIndex = IDObject - 1;
                if (removedEntityIndex >= super::_sparse.size()) { //needed for scene.removeEntity(), as it iterates over all systems and some might not have the entity in them
                    return false;
                }
                const uint& removedComponentID = super::_sparse[removedEntityIndex];
                if (removedComponentID == 0)
                    return false;
                super::_sparse[removedEntityIndex] = 0;
                if (removedEntityIndex >= super::_maxLast) { 
                    super::_maxLast = 0;
                    for (uint i = super::_sparse.size(); i-- > 0;) { 
                        if (super::_sparse[i] > 0) { 
                            super::_maxLast = i; 
                            break; 
                        } 
                    } 
                }
                if (_dense.size() > 1) {
                    using std::swap;
                    swap(_dense[removedComponentID - 1], _dense[_dense.size() - 1]);
                    super::_sparse[super::_maxLast] = removedComponentID;
                }
                _dense.pop_back();
                return true;
            }
            inline T* _get(const uint& _IDObject) {
                const uint& sparseIndex = _IDObject - 1;
                const uint& sparseSize  = super::_sparse.size();
                if (sparseSize == 0 || sparseIndex >= sparseSize || super::_sparse[sparseIndex] == 0)
                    return nullptr;
                return &(_dense[super::_sparse[sparseIndex] - 1]);
            }
    };
};
};

#endif