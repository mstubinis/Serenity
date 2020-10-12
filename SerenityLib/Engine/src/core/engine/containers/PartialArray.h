#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_ARRAY_H
#define ENGINE_CONTAINERS_PARTIAL_ARRAY_H

namespace Engine {
    template<class T, size_t S> 
    class partial_array final {
        private:
            std::array<T, S> m_Array;
            size_t           m_Count = 0U;
        public:
            partial_array() = default;
            partial_array(const partial_array& other)                = default;
            partial_array& operator=(const partial_array& other)     = default;
            partial_array(partial_array&& other) noexcept            = default;
            partial_array& operator=(partial_array&& other) noexcept = default;

            inline void fill(const T& item) noexcept { m_Array.fill(item); }
            inline CONSTEXPR size_t size() const noexcept { return m_Count; }
            inline CONSTEXPR size_t max_size() const noexcept { return m_Array.size(); }
            inline CONSTEXPR size_t capacity() const noexcept { return m_Array.size(); }
            inline CONSTEXPR const T* data() const noexcept { return m_Array.data(); }
            inline void clear(const T& item) noexcept { fill(item); m_Count = 0U; }
            inline void clear() noexcept { m_Count = 0U; }

            inline void swap(size_t indexOne, size_t indexTwo) noexcept { std::swap(m_Array[indexOne], m_Array[indexTwo]); }
            inline void swap(partial_array& otherArray) noexcept { std::swap(m_Count, otherArray.m_Count); m_Array.swap(otherArray.m_Array); }

            inline T& operator[](size_t index) noexcept { return m_Array[index]; }
            inline const T& operator[](size_t index) const noexcept { return m_Array[index]; }

            inline void push(const T& item) noexcept { m_Array[m_Count++] = item; }
            inline void push(T&& item) noexcept { m_Array[m_Count++] = std::move(item); }
            template<typename... ARGS> 
            inline void emplace_push(ARGS&&... args) { m_Array[m_Count++] = T(std::forward<ARGS>(args)...); }

            inline void pop() noexcept { m_Array[m_Count - 1U] = T(); --m_Count; }

            typename inline std::array<T, S>::iterator begin() { return m_Array.begin(); }
            typename inline std::array<T, S>::iterator end() { return m_Array.end(); }
            typename inline std::array<T, S>::const_iterator begin() const { return m_Array.begin(); }
            typename inline std::array<T, S>::const_iterator end() const { return m_Array.end(); }
    };
}

#endif