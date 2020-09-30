#pragma once
#ifndef ENGINE_CONTAINERS_PARTIAL_VECTOR_POD_H
#define ENGINE_CONTAINERS_PARTIAL_VECTOR_POD_H

namespace Engine {
    template<typename T>
    class partial_vector_pod final {
        private:
            std::vector<T> m_Items;
            size_t         m_Count = 0U;
        public:
            partial_vector_pod() = default;
            partial_vector_pod(size_t capacity) {
                resize(capacity);
            }
            partial_vector_pod(const partial_vector_pod& other)                = default;
            partial_vector_pod& operator=(const partial_vector_pod& other)     = default;
            partial_vector_pod(partial_vector_pod&& other) noexcept            = default;
            partial_vector_pod& operator=(partial_vector_pod&& other) noexcept = default;
            ~partial_vector_pod() = default;

            inline void resize(size_t capacity, T&& item) {
                m_Items.resize(capacity, item);
            }
            inline void resize(size_t capacity) {
                m_Items.resize(capacity);
            }

            //inline void insert(iterator location, move_iterator first, move_iterator last) {
            //    m_Items.insert(location, first, last);
            //}

            inline CONSTEXPR size_t size() const noexcept { return m_Count; }
            inline CONSTEXPR size_t capacity() const noexcept { return m_Items.size(); }
            inline CONSTEXPR const T* data() const noexcept { return m_Items.data(); }
            inline void clear() noexcept { m_Count = 0U; }

            inline void swap(size_t indexOne, size_t indexTwo) noexcept { std::swap(m_Items[indexOne], m_Items[indexTwo]); }
            inline void swap(partial_vector_pod& otherVector) noexcept { std::swap(m_Count, otherVector.m_Count); m_Items.swap(otherVector.m_Items); }

            inline T& operator[](size_t index) noexcept { return m_Items[index]; }
            inline const T& operator[](size_t index) const noexcept { return m_Items[index]; }

            inline void push(const T& item) noexcept { 
                if (m_Count >= capacity()) {
                    return;
                }
                m_Items[m_Count++] = item; 
            }
            inline void push(T&& item) noexcept { 
                if (m_Count >= capacity()) {
                    return;
                }
                m_Items[m_Count++] = std::move(item); 
            }
            template<typename... ARGS>
            inline void emplace_push(ARGS &&... args) { m_Items[m_Count++] = T(std::forward<ARGS>(args)...); }

            inline void pop() noexcept { m_Items[m_Count - 1U] = T(); --m_Count; }

            typename inline std::vector<T>::iterator begin() { return m_Items.begin(); }
            typename inline std::vector<T>::iterator end() { return m_Items.begin() + m_Count; }
            typename inline std::vector<T>::const_iterator begin() const { return m_Items.begin(); }
            typename inline std::vector<T>::const_iterator end() const { return m_Items.begin() + m_Count; }
    };
};

#endif