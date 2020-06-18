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
                std::uint32_t               m_MaxLastIndex = 0U;
                std::vector<std::uint32_t>  m_Sparse;

                sparse_set_base() = delete;
            public:
                sparse_set_base(std::uint32_t initial_capacity) {
                    if (initial_capacity > 0U) {
                        m_Sparse.reserve(initial_capacity);
                    }
                }
                virtual ~sparse_set_base() {
                    m_MaxLastIndex = 0U;
                    m_Sparse.clear();
                }
                virtual bool remove(std::uint32_t id) {
                    return false;
                }
                virtual void reserve(std::uint32_t amount) {
                }
            };
    };
    template <typename T>
    class sparse_set : public Engine::priv::sparse_set_base {
        using super = Engine::priv::sparse_set_base;
        private:
            std::vector<T> m_Dense;
        public:
            sparse_set() : super(0U) {
            }
            sparse_set(std::uint32_t initial_capacity) : super(initial_capacity) {
                m_Dense.reserve(initial_capacity);
            }
            virtual ~sparse_set() {
                m_Dense.clear();
            }

            void reserve(std::uint32_t amount) override {
                if (super::m_Sparse.size() == super::m_Sparse.capacity()) {
                    super::m_Sparse.reserve(super::m_Sparse.capacity() + amount);
                }
                if (m_Dense.size() == m_Dense.capacity()) {
                    m_Dense.reserve(m_Dense.capacity() + amount);
                }
            }
            template<typename... ARGS> 
            T* add(std::uint32_t id, ARGS&&... args) {
                std::uint32_t sparseIndex = id - 1U;
                if (static_cast<size_t>(sparseIndex) >= super::m_Sparse.size()) {
                    super::m_Sparse.resize(static_cast<size_t>(sparseIndex) + 1U, 0U);
                }
                if (super::m_Sparse[sparseIndex] != 0U) {
                    return nullptr;
                }
                m_Dense.emplace_back(std::forward<ARGS>(args)...);
                super::m_Sparse[sparseIndex] = static_cast<std::uint32_t>(m_Dense.size());
                super::m_MaxLastIndex        = sparseIndex;
                return &m_Dense[super::m_Sparse[sparseIndex] - 1];
            }
            //TODO: this entire function needs a serious look at
            bool remove(std::uint32_t id) override {
                auto removedEntityIndex = id - 1;
                if (removedEntityIndex >= super::m_Sparse.size()) {
                    return false;
                }
                auto removedComponentID = super::m_Sparse[removedEntityIndex];
                if (removedComponentID == 0U) {
                    return false;
                }
                super::m_Sparse[removedEntityIndex] = 0U;
                super::m_MaxLastIndex = 0U;
                std::uint32_t max_val_ = 0U;
                for (size_t i = super::m_Sparse.size(); i-- > 0;) {
                    if (super::m_Sparse[i] > 0) {
                        if (super::m_Sparse[i] > max_val_) {
                            super::m_MaxLastIndex = static_cast<std::uint32_t>(i);
                            max_val_ = super::m_Sparse[i];
                        }
                    }
                }
                if (m_Dense.size() > 1) {
                    auto firstIndex = removedComponentID - 1U;
                    auto lastIndex  = m_Dense.size() - 1U;
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
            T* get(std::uint32_t id) const {
                auto entityIndexInSparse = id - 1U;
                auto sparseSize          = super::m_Sparse.size();
                if (sparseSize == 0 || entityIndexInSparse >= sparseSize || super::m_Sparse[entityIndexInSparse] == 0U) {
                    return nullptr;
                }
                auto retIndex = super::m_Sparse[entityIndexInSparse] - 1U;
                if (retIndex >= m_Dense.size()) {
                    return nullptr;
                }
                auto* ret = &m_Dense[retIndex];
                return const_cast<T*>(ret);
            }


            T& operator[](size_t index) {
                return m_Dense[index];
            }
            const T& operator[](size_t index) const {
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