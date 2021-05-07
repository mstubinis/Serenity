#pragma once
#ifndef ENGINE_CONTAINERS_SPARSE_SET_H
#define ENGINE_CONTAINERS_SPARSE_SET_H

#include <vector>
#include <serenity/system/Macros.h>

namespace Engine {
    namespace priv {
        class sparse_set_base {
            protected:
                std::vector<uint32_t>  m_Sparse;

                sparse_set_base() = default;
            public:
                sparse_set_base(uint32_t initial_capacity) {
                    m_Sparse.reserve(initial_capacity);
                }

                sparse_set_base(const sparse_set_base&)                = default;
                sparse_set_base& operator=(const sparse_set_base&)     = default;
                sparse_set_base(sparse_set_base&&) noexcept            = default;
                sparse_set_base& operator=(sparse_set_base&&) noexcept = default;

                inline bool has(uint32_t id) noexcept { 
                    return (m_Sparse.size() < id) ? false : m_Sparse[id - 1] != 0; 
                }

                virtual ~sparse_set_base() = default;
                virtual bool remove(uint32_t id) {
                    return false;
                }
                virtual void reserve(uint32_t amount) {}
                virtual void clear() {
                    m_Sparse.clear();
                }
            };
    };
    template <typename T>
    class sparse_set : public Engine::priv::sparse_set_base {
        struct Entry final {
            T        comp;
            uint32_t idx;

            Entry() = delete;
            Entry(const Entry&)                = delete;
            Entry& operator=(const Entry&)     = delete;
            Entry(Entry&&) noexcept            = default;
            Entry& operator=(Entry&&) noexcept = default;

            template<class ... ARGS>
            Entry(uint32_t idx_, ARGS&&... args) 
                : comp{ std::forward<ARGS>(args)... }
                , idx { idx_ }
            {}
        };
        using super     = Engine::priv::sparse_set_base;
        using container = std::vector<Entry>;
        private:
            container m_Dense;
        public:
            sparse_set() = default;
            sparse_set(uint32_t initial_capacity) 
                : super{ initial_capacity }
            {
                m_Dense.reserve(initial_capacity);
            }

            sparse_set(const sparse_set&)                = default;
            sparse_set& operator=(const sparse_set&)     = default;
            sparse_set(sparse_set&&) noexcept            = default;
            sparse_set& operator=(sparse_set&&) noexcept = default;

            virtual ~sparse_set() = default;
            void clear() override {
                m_Dense.clear();
                super::clear();
            }

            void reserve(uint32_t amount) override {
                if (super::m_Sparse.size() == super::m_Sparse.capacity()) {
                    super::m_Sparse.reserve(super::m_Sparse.capacity() + amount);
                }
                if (m_Dense.size() == m_Dense.capacity()) {
                    m_Dense.reserve(m_Dense.capacity() + amount);
                }
            }
            template<class ... ARGS> 
            constexpr T* add(uint32_t id, ARGS&&... args) {
                uint32_t sparseIndex = id - 1;
                if (static_cast<size_t>(sparseIndex) >= super::m_Sparse.size()) {
                    super::m_Sparse.resize(static_cast<size_t>(sparseIndex) + 1, 0);
                }
                if (super::m_Sparse[sparseIndex] != 0) {
                    return nullptr;
                }
                m_Dense.emplace_back(sparseIndex, std::forward<ARGS>(args)...);
                super::m_Sparse[sparseIndex] = static_cast<uint32_t>(m_Dense.size());
                return &m_Dense[super::m_Sparse[sparseIndex] - 1].comp;
            }
            //TODO: this entire function needs a serious look at
            constexpr bool remove(uint32_t id) override {
                auto entityIdx = id - 1;
                if (entityIdx >= super::m_Sparse.size()) {
                    return false;
                }
                auto removedComponentID = super::m_Sparse[entityIdx];
                if (removedComponentID == 0) {
                    return false;
                }
                auto removedDenseIdx = removedComponentID - 1;
                auto lastIndex = m_Dense.size() - 1;
                super::m_Sparse[m_Dense[lastIndex].idx] = removedComponentID;
                super::m_Sparse[entityIdx] = 0;
                std::swap(m_Dense[removedDenseIdx], m_Dense[lastIndex]);
                m_Dense.pop_back();
                return true;
            }
            constexpr T* get(uint32_t id) {
                auto entityIndexInSparse = id - 1;
                auto sparseSize          = super::m_Sparse.size();
                if (sparseSize == 0 || entityIndexInSparse >= sparseSize || super::m_Sparse[entityIndexInSparse] == 0) {
                    return nullptr;
                }
                auto retIndex = super::m_Sparse[entityIndexInSparse] - 1;
                if (retIndex >= m_Dense.size()) {
                    return nullptr;
                }
                return &m_Dense[retIndex].comp;
            }

            inline T& operator[](size_t index) { return m_Dense[index].comp; }
            inline const T& operator[](size_t index) const noexcept { return m_Dense[index].comp; }
            inline constexpr size_t size() const noexcept { return m_Dense.size(); }
            inline constexpr container& data() noexcept { return m_Dense; }

            BUILD_TEMPLATE_BEGIN_END_ITR_CLASS_MEMBERS(container, m_Dense)
    };
};

#endif