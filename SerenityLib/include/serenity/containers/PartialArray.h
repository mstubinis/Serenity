#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_ARRAY_H
#define ENGINE_CONTAINERS_PARTIAL_ARRAY_H

#include <array>
#include <serenity/system/Macros.h>

namespace Engine {
    template<class T, size_t S> 
    class partial_array final {
        using ArrayType = std::array<T, S>;
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
            inline void clear(const T& item) noexcept { fill(item); m_Count = 0; }
            inline void clear() noexcept { m_Count = 0; }

            inline void swap(size_t indexOne, size_t indexTwo) noexcept { std::swap(m_Array[indexOne], m_Array[indexTwo]); }
            inline void swap(partial_array& otherArray) noexcept { std::swap(m_Count, otherArray.m_Count); m_Array.swap(otherArray.m_Array); }

            inline T& operator[](size_t index) noexcept { return m_Array[index]; }
            inline const T& operator[](size_t index) const noexcept { return m_Array[index]; }

            inline void push(const T& item) noexcept { m_Array[m_Count++] = item; }
            inline void push(T&& item) noexcept { m_Array[m_Count++] = std::move(item); }
            template<typename... ARGS> 
            inline void emplace_push(ARGS&&... args) { m_Array[m_Count++] = T(std::forward<ARGS>(args)...); }

            inline void pop() noexcept { --m_Count; }

            BUILD_TEMPLATE_BEGIN_END_ITR_CLASS_MEMBERS(ArrayType, m_Array)
    };
}

#endif