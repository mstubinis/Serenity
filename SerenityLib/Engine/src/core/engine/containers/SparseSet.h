#pragma once
#ifndef ENGINE_CONTAINERS_SPARSE_SET_H
#define ENGINE_CONTAINERS_SPARSE_SET_H

#include <utility>

#include <ecs/ECSRegistry.h>
#include <iostream>

namespace Engine {
    namespace priv {
        class sparse_set_base {
            protected:
                unsigned int                            m_MaxLastIndex = 0;
                std::vector<unsigned int>               m_Sparse;

                sparse_set_base() = delete;
            public:
                sparse_set_base(const unsigned int initial_capacity) {
                    if (initial_capacity > 0) {
                        m_Sparse.reserve(initial_capacity);
                    }
                }
                virtual ~sparse_set_base() {
                    m_MaxLastIndex = 0;
                    m_Sparse.clear();
                }
                virtual bool remove(const unsigned int id) {
                    return false;
                }
                virtual void reserve(const unsigned int amount) {
                }
            };
    };
    template <typename T>
    class sparse_set : public Engine::priv::sparse_set_base {
        using super = Engine::priv::sparse_set_base;
        private:
            std::vector<T> m_Dense;
        public:
            sparse_set() : super(0) {
            }
            sparse_set(const unsigned int initial_capacity) : super(initial_capacity) {
                m_Dense.reserve(initial_capacity);
            }
            virtual ~sparse_set() {
                m_Dense.clear();
            }

            void reserve(const unsigned int amount) override {
                if (super::m_Sparse.size() == super::m_Sparse.capacity()) {
                    super::m_Sparse.reserve(super::m_Sparse.capacity() + amount);
                }
                if (m_Dense.size() == m_Dense.capacity()) {
                    m_Dense.reserve(m_Dense.capacity() + amount);
                }
            }
            template<typename... ARGS> 
            T* add(const unsigned int id, ARGS&& ... args) {
                const unsigned int sparseIndex = id - 1U;
                if (static_cast<size_t>(sparseIndex) >= super::m_Sparse.size()) {
                    super::m_Sparse.resize(static_cast<size_t>(sparseIndex) + 1U, 0);
                }
                if (super::m_Sparse[sparseIndex] != 0) {
                    return nullptr;
                }
                m_Dense.emplace_back(std::forward<ARGS>(args)...);

                super::m_Sparse[sparseIndex] = static_cast<unsigned int>(m_Dense.size());
                super::m_MaxLastIndex        = sparseIndex;
                return &m_Dense[super::m_Sparse[sparseIndex] - 1];
            }
            //TODO: this entire function needs a serious look at
            bool remove(const unsigned int id) override {
                const auto removedEntityIndex = id - 1;
                if (removedEntityIndex >= super::m_Sparse.size()) {
                    return false;
                }
                const auto removedComponentID = super::m_Sparse[removedEntityIndex];
                if (removedComponentID == 0) {
                    return false;
                }
                super::m_Sparse[removedEntityIndex] = 0;
                super::m_MaxLastIndex = 0;
                unsigned int max_val_ = 0;
                for (size_t i = super::m_Sparse.size(); i-- > 0;) {
                    if (super::m_Sparse[i] > 0) {
                        if (super::m_Sparse[i] > max_val_) {
                            super::m_MaxLastIndex = static_cast<unsigned int>(i);
                            max_val_ = super::m_Sparse[i];
                        }
                    }
                }
                if (m_Dense.size() > 1) {
                    const auto firstIndex = removedComponentID - 1;
                    const auto lastIndex  = m_Dense.size() - 1;
                    if (firstIndex != lastIndex) {
                        if (firstIndex < lastIndex) {
                            std::swap(m_Dense[firstIndex], m_Dense[lastIndex]);
                            super::m_Sparse[super::m_MaxLastIndex] = removedComponentID;
                        }
                    }
                }
                m_Dense.pop_back();
                return true;
            }
            T* get(const unsigned int id) const {
                const auto entityIndexInSparse = id - 1;
                const auto sparseSize          = super::m_Sparse.size();
                if (sparseSize == 0 || entityIndexInSparse >= sparseSize || super::m_Sparse[entityIndexInSparse] == 0) {
                    return nullptr;
                }
                const auto retIndex = super::m_Sparse[entityIndexInSparse] - 1;
                if (retIndex >= m_Dense.size()) {
                    return nullptr;
                }
                auto* ret = &m_Dense[retIndex];
                return const_cast<T*>(ret);
            }


            T& operator[](const size_t& index) {
                return m_Dense[index];
            }
            const T& operator[](const size_t& index) const {
                return m_Dense[index];
            }
            size_t size() const {
                return m_Dense.size();
            }
            std::vector<T>& data() {
                return m_Dense;
            }
            typename std::vector<T>::iterator begin() {
                return m_Dense.begin();
            }
            typename std::vector<T>::iterator end() {
                return m_Dense.end();
            }
            typename std::vector<T>::const_iterator begin() const {
                return m_Dense.begin();
            }
            typename std::vector<T>::const_iterator end() const {
                return m_Dense.end();
            }
    };
};

#endif