#pragma once
#ifndef ENGINE_ECS_SPARSE_SET_H
#define ENGINE_ECS_SPARSE_SET_H

#include <utility>

namespace Engine::priv {
    template <typename...>      class SparseSet;
    template <typename TID>     class SparseSet<TID> {
        protected:
            size_t                            maxLastIndex;
            std::vector<size_t>               sparse; //maps entity ID to component Index in dense
        public:
            SparseSet() { 
                maxLastIndex = 0;
                sparse.reserve(5000);
            }
            virtual ~SparseSet() { 
                maxLastIndex = 0;
                sparse.clear();
            }
            virtual const bool remove(const unsigned int& entityID) { 
                return false;  
            }
            virtual void reserveMore(const unsigned int amount) {
            }
    };
    template <typename TID, typename T> class SparseSet<TID, T> : public SparseSet<TID> {
        using super = SparseSet<TID>;
        private:
            std::vector<T> dense;

        public:
			std::vector<T>& pool() { return dense; }

            SparseSet() {
                dense.reserve(5000);
            }
            SparseSet(const SparseSet& other) noexcept            = delete;
            SparseSet& operator=(const SparseSet& other) noexcept = delete;
            SparseSet(SparseSet&& other) noexcept                 = delete;
            SparseSet& operator=(SparseSet&& other) noexcept      = delete;
            virtual ~SparseSet() {
                dense.clear();
            }

            void reserveMore(const unsigned int amount) override {
                if (super::sparse.size() > super::sparse.capacity() - 1) {
                    super::sparse.reserve(super::sparse.capacity() + amount);
                }
                if (dense.size() > dense.capacity() - 1) {
                    dense.reserve(dense.capacity() + amount);
                }
            }
            template<typename... ARGS> T* add(const unsigned int& entityID, ARGS&&... args) {
                const auto sparseIndex     = entityID - 1;
                if (sparseIndex >= super::sparse.size()) {
                    super::sparse.resize(static_cast<size_t>(sparseIndex) + 1, 0);
                }
                if (super::sparse[sparseIndex] != 0) {
                    return nullptr;
                }
                dense.emplace_back(  std::forward<ARGS>(args)...  );
                super::sparse[sparseIndex] = dense.size();
                super::maxLastIndex        = sparseIndex;
                auto* ret                  = &dense[super::sparse[sparseIndex] - 1];
                return ret;
            }
            //TODO: this entire function needs a serious look at
            const bool remove(const unsigned int& entityID) override {
                const auto removedEntityIndex = entityID - 1;
                if (removedEntityIndex >= super::sparse.size()) { //needed for scene.removeEntity(), as it iterates over all systems and some might not have the entity in them
                    return false;
                }
                const auto removedComponentID = super::sparse[removedEntityIndex];
                if (removedComponentID == 0) {
                    return false;
                }
                super::sparse[removedEntityIndex] = 0;
                super::maxLastIndex = 0;
                size_t maxValue = 0;
                for (size_t i = super::sparse.size(); i-- > 0;) { 
                    if (super::sparse[i] > 0) {
                        if (super::sparse[i] > maxValue) {
                            super::maxLastIndex = i;
                            maxValue = super::sparse[i];
                        }
                    } 
                } 
                if (dense.size() > 1) {
                    const auto firstIndex = removedComponentID - 1;
                    const auto lastIndex = dense.size() - 1;
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
            T* get(const unsigned int& entityID) {
                const auto entityIndexInSparse = entityID - 1;
                const auto sparseSize  = super::sparse.size();
                if (sparseSize == 0 || entityIndexInSparse >= sparseSize || super::sparse[entityIndexInSparse] == 0) {
                    return nullptr;
                }
                const auto retIndex = super::sparse[entityIndexInSparse] - 1;
                if (retIndex >= dense.size()) {
                    return nullptr;
                }
                return &dense[retIndex];
            }
    };
};

#endif