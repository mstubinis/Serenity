#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_ARRAY_H
#define ENGINE_CONTAINERS_PARTIAL_ARRAY_H

#include <array>
#include <serenity/system/Macros.h>

namespace Engine {
    template<class T, size_t FIXED_SIZE> 
    class partial_array final {
        using ArrayType = std::array<T, FIXED_SIZE>;
        private:
            ArrayType m_Array;
            size_t    m_Count = 0;
        public:
            partial_array() = default;
            partial_array(const partial_array&)                = default;
            partial_array& operator=(const partial_array&)     = default;
            partial_array(partial_array&&) noexcept            = default;
            partial_array& operator=(partial_array&&) noexcept = default;

            inline void fill(const T& item) noexcept { m_Array.fill(item); }
            inline constexpr size_t size() const noexcept { return m_Count; }
            inline constexpr size_t max_size() const noexcept { return m_Array.size(); }
            inline constexpr size_t capacity() const noexcept { return m_Array.size(); }
            inline constexpr const T* data() const noexcept { return m_Array.data(); }

            void clear(const T& item) noexcept { 
                fill(item); 
                m_Count = 0; 
            }

            inline void clear() noexcept { m_Count = 0; }

            inline void swap(size_t indexOne, size_t indexTwo) noexcept { 
                std::swap(m_Array[indexOne], m_Array[indexTwo]); 
            }
            void swap(partial_array& otherArray) noexcept { 
                std::swap(m_Count, otherArray.m_Count); 
                std::swap(m_Array, otherArray.m_Array);
            }

            inline T& operator[](size_t index) noexcept { 
                return m_Array[index];
            }
            inline const T& operator[](size_t index) const noexcept {
                return m_Array[index]; 
            }

            inline void push(const T& item) noexcept { 
                ASSERT(m_Count < FIXED_SIZE, __FUNCTION__ << "(): partial array reached capacity!");
                m_Array[m_Count++] = item; 
            }
            inline void push(T&& item) noexcept { 
                ASSERT(m_Count < FIXED_SIZE, __FUNCTION__ << "(): partial array reached capacity!");
                m_Array[m_Count++] = std::move(item); 
            }
            template<class ... ARGS> 
            inline void emplace_push(ARGS&&... args) { 
                ASSERT(m_Count < FIXED_SIZE, __FUNCTION__ << "(): partial array reached capacity!");
                m_Array[m_Count++] = T(std::forward<ARGS>(args)...); 
            }

            inline void pop() noexcept { 
                ASSERT(m_Count >= 1, __FUNCTION__ << "(): partial array is already empty!");
                --m_Count;
            }

            //notice the end() functions use m_Count instead of end()
            using itr = typename ArrayType::iterator;
            using itr_const = typename ArrayType::const_iterator;
            inline itr begin() noexcept { return m_Array.begin(); }
            inline itr_const begin() const noexcept { return m_Array.begin(); }
            inline itr end() noexcept { return m_Array.begin() + m_Count; }
            inline itr_const end() const noexcept { return m_Array.begin() + m_Count; }
            inline const itr_const cbegin() const noexcept { return m_Array.cbegin(); }
            inline const itr_const cend() const noexcept { return m_Array.cbegin() + m_Count; }
    };
}

#endif