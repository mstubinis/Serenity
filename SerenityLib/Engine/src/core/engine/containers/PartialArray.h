#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_ARRAY_H
#define ENGINE_CONTAINERS_PARTIAL_ARRAY_H

namespace Engine {
    template<class T, size_t S> class partial_array {
        private:
            std::array<T, S> m_Array;
            size_t           m_Count = 0U;
        public:
            partial_array() = default;
            virtual ~partial_array() = default;

            void fill(const T item) noexcept {
                m_Array.fill(item);
            }
            inline CONSTEXPR size_t size() const noexcept {
                return m_Count;
            }
            inline CONSTEXPR size_t max_size() const noexcept {
                return m_Array.size();
            }
            inline CONSTEXPR size_t capacity() const noexcept {
                return m_Array.max_size();
            }
            inline CONSTEXPR const T* data() const noexcept {
                return m_Array.data();
            }
            void clear(const T item) noexcept {
                fill(item);
                m_Count = 0U;
            }
            inline void clear() noexcept {
                m_Count = 0U;
            }

            T& operator[](unsigned int index) = delete;
            const T& operator[](unsigned int index) const = delete;

            inline T& get(unsigned int index) noexcept {
                return m_Array[index];
            }
            void put(const T item) noexcept {
                m_Array[m_Count] = item;
                ++m_Count;
            }
            template<typename... ARGS> void emplace_put(ARGS&&... args) {
                m_Array[m_Count] = T(std::forward<ARGS>(args)...);
                ++m_Count;
            }

            void pop() noexcept {
                m_Array[m_Count - 1U] = T();
                --m_Count;
            }

            typename std::array<T, S>::iterator begin() { return m_Array.begin(); }
            typename std::array<T, S>::iterator end() { return m_Array.end(); }
            typename std::array<T, S>::const_iterator begin() const { return m_Array.begin(); }
            typename std::array<T, S>::const_iterator end() const { return m_Array.end(); }
    };
}

#endif