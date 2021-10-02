#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_VECTOR_POD_H
#define ENGINE_CONTAINERS_PARTIAL_VECTOR_POD_H

#include <vector>
#include <serenity/system/Macros.h>

namespace Engine {
    template<class T, class ALLOCATOR = ::std::allocator<T>>
    class partial_vector_pod final {
        public:
            using ContainerType = ::std::vector<T, ALLOCATOR>;
        private:
            ContainerType  m_Items;
            size_t         m_Count = 0;
        public:
            partial_vector_pod() = default;
            partial_vector_pod(size_t capacity) {
                resize(capacity);
            }
            partial_vector_pod(const partial_vector_pod&)                = default;
            partial_vector_pod& operator=(const partial_vector_pod&)     = default;
            partial_vector_pod(partial_vector_pod&&) noexcept            = default;
            partial_vector_pod& operator=(partial_vector_pod&&) noexcept = default;

            inline void resize(size_t capacity, T&& item) {
                m_Items.resize(capacity, item);
            }
            inline void resize(size_t capacity) {
                m_Items.resize(capacity);
            }
            inline constexpr size_t size() const noexcept { return m_Count; }
            inline constexpr size_t capacity() const noexcept { return m_Items.size(); }
            inline constexpr const T* data() const noexcept { return m_Items.data(); }
            inline void clear() noexcept { m_Count = 0; }

            inline void swap(size_t indexOne, size_t indexTwo) noexcept { std::swap(m_Items[indexOne], m_Items[indexTwo]); }
            inline void swap(partial_vector_pod& otherVector) noexcept { std::swap(m_Count, otherVector.m_Count); m_Items.swap(otherVector.m_Items); }

            inline T& operator[](size_t index) noexcept { return m_Items[index]; }
            inline const T& operator[](size_t index) const noexcept { return m_Items[index]; }

            inline T& front() noexcept { return m_Items[0]; }
            inline const T& front() const noexcept { return m_Items[0]; }
            inline T& back() noexcept { return m_Items[size() - 1]; }
            inline const T& back() const noexcept { return m_Items[size() - 1]; }

            int push(T&& item) noexcept {
                if (m_Count >= capacity()) {
                    return -1;
                }
                m_Items[m_Count] = std::forward<T>(item); 
                int res = m_Count++;
                return res;
            }
            template<class ... ARGS>
            int emplace_push(ARGS&&... args) noexcept {
                m_Items[m_Count] = T(std::forward<ARGS>(args)...);
                int res = m_Count++;
                return res;
            }

            inline void pop() noexcept {
                --m_Count;
            }

            void swap_and_pop(size_t index) {
                m_Items[index] = std::move(m_Items.back());
                pop();
            }
            int insert(size_t index, T&& item) {
                if (index >= m_Count) {
                    return -1;
                }
                m_Items[index] = std::forward<T>(item);
                return static_cast<int>(index);
            }
            //notice the end() functions use m_Count instead of end()
            inline typename ContainerType::iterator begin() noexcept { return m_Items.begin(); }
            inline typename ContainerType::const_iterator begin() const noexcept { return m_Items.begin(); }
            inline typename ContainerType::iterator end() noexcept { return m_Items.begin() + m_Count; }
            inline typename ContainerType::const_iterator end() const noexcept { return m_Items.begin() + m_Count; }
            inline typename const ContainerType::const_iterator cbegin() const noexcept { return m_Items.cbegin(); }
            inline typename const ContainerType::const_iterator cend() const noexcept { return m_Items.cbegin() + m_Count; }
    };
};

#endif