#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_ARRAY_H
#define ENGINE_CONTAINERS_PARTIAL_ARRAY_H

#include <array>

namespace Engine {
    template<class T, size_t S> class partial_array {
        private:
            std::array<T, S> m_Array;
            size_t           m_Count = 0U;
        public:
            partial_array() {

            }
            virtual ~partial_array() {

            }

            void fill(const T item) {
                m_Array.fill(item);
            }
            constexpr size_t size() const noexcept {
                return m_Count;
            }
            constexpr size_t max_size() const noexcept {
                return m_Array.size();
            }
            constexpr size_t capacity() const noexcept {
                return m_Array.max_size();
            }
            constexpr const T* data() const noexcept {
                return m_Array.data();
            }
            void clear(const T item) {
                fill(item);
                m_Count = 0U;
            }
            void clear() {
                m_Count = 0U;
            }


            T& operator[](unsigned int index) = delete;
            const T& operator[](unsigned int index) const = delete;


            T& get(unsigned int index) {
                return m_Array[index];
            }
            void put(const T item) {
                m_Array[m_Count] = item;
                ++m_Count;
            }
            template<typename... ARGS> void emplace_put(ARGS&&... args) {
                m_Array[m_Count] = T(std::forward<ARGS>(args)...);
                ++m_Count;
            }

            void pop() {
                m_Array[m_Count - 1U] = T();
                --m_Count;
            }

            typename std::array<T, S>::iterator begin() {
                return m_Array.begin();
            }
            typename std::array<T, S>::iterator end() {
                return m_Array.end();
            }
            typename std::array<T, S>::const_iterator begin() const {
                return m_Array.begin();
            }
            typename std::array<T, S>::const_iterator end() const {
                return m_Array.end();
            }
    };
}

#endif