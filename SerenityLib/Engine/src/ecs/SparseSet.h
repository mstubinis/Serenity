#pragma once
#ifndef ENGINE_ECS_SPARSE_SET_H
#define ENGINE_ECS_SPARSE_SET_H

#include <algorithm> //std::swap (until C++11)
#include <utility>   //std::swap (since C++11)
#include <core/engine/utils/Utils.h>
#include <ecs/Entity.h>

#include <iostream>

//MAX_ENTITIES = 2097152;
namespace Engine {
namespace epriv {

    template <typename...>      class SparseSet;
    template <typename TID>     class SparseSet<TID> {
        protected:
            uint                              maxLastIndex;
            std::vector<uint>                 sparse; //maps entity ID to component Index in dense
        public:
            SparseSet() { 
                maxLastIndex = 0;
            }
            virtual ~SparseSet() { 
                maxLastIndex = 0;
                sparse.clear();
            }
            virtual inline const bool _remove(const uint& entityID) { return false;  }
            virtual inline void reserveMore(const uint amount) { }
    };
    template <typename TID, typename T> class SparseSet<TID, T> : public SparseSet<TID> {
        using super = SparseSet<TID>;
        private:
            std::vector<T> dense;
        public:
			std::vector<T>& pool() { return dense; }

            SparseSet() {}
            SparseSet(const SparseSet& other) noexcept            = delete;
            SparseSet& operator=(const SparseSet& other) noexcept = delete;
            SparseSet(SparseSet&& other) noexcept                 = delete;
            SparseSet& operator=(SparseSet&& other) noexcept      = delete;
            virtual ~SparseSet()                                  = default;

            inline void reserveMore(const uint amount) {
                if (super::sparse.size() > super::sparse.capacity() - 1) {
                    super::sparse.reserve(super::sparse.capacity() + amount);
                }
                if (dense.size() > dense.capacity() - 1) {
                    dense.reserve(dense.capacity() + amount);
                }
            }

            template<typename... ARGS> inline T* _add(const uint& entityID, ARGS&&... args) {
                const uint sparseIndex = entityID - 1;
                if (sparseIndex >= super::sparse.size()) {
                    super::sparse.resize(sparseIndex + 1, 0);
                }
                if (super::sparse[sparseIndex] != 0) {
                    return nullptr;
                }
                dense.emplace_back(std::forward<ARGS>(args)...);
                super::sparse[sparseIndex] = dense.size();
                super::maxLastIndex = sparseIndex;
                auto* ret = &dense[super::sparse[sparseIndex] - 1];
                return ret;
            }
            //TODO: this entire function needs a serious look at
            inline const bool _remove(const uint& entityID) {
                const uint removedEntityIndex = entityID - 1;
                if (removedEntityIndex >= super::sparse.size()) { //needed for scene.removeEntity(), as it iterates over all systems and some might not have the entity in them
                    return false;
                }
                const uint removedComponentID = super::sparse[removedEntityIndex];
                if (removedComponentID == 0) {
                    return false;
                }
                super::sparse[removedEntityIndex] = 0;
                super::maxLastIndex = 0;
                uint maxValue = 0;
                for (uint i = super::sparse.size(); i-- > 0;) { 
                    if (super::sparse[i] > 0) {
                        if (super::sparse[i] > maxValue) {
                            super::maxLastIndex = i;
                            maxValue = super::sparse[i];
                        }
                    } 
                } 
                if (dense.size() > 1) {
                    const uint firstIndex = removedComponentID - 1;
                    const uint lastIndex = dense.size() - 1;
                    if (firstIndex != lastIndex) {
                        if (firstIndex < lastIndex) {
                            std::swap(dense[firstIndex], dense[lastIndex]);
                            super::sparse[super::maxLastIndex] = removedComponentID;
                        }
                    }
                }
                dense.pop_back();
                return true;
            }
            inline T* _get(const uint& entityID) {
                const uint entityIndexInSparse = entityID - 1;
                const uint sparseSize  = super::sparse.size();
                if (sparseSize == 0 || entityIndexInSparse >= sparseSize || super::sparse[entityIndexInSparse] == 0) {
                    return nullptr;
                }
                const uint retIndex = super::sparse[entityIndexInSparse] - 1;
                if (retIndex >= dense.size()) {
                    return nullptr;
                }
                return &dense[retIndex];
            }
    };
};
};

#endif