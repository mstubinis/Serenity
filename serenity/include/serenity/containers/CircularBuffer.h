#pragma once
#ifndef ENGINE_CONTAINERS_CIRCULAR_BUFFER_H
#define ENGINE_CONTAINERS_CIRCULAR_BUFFER_H

#include <atomic>
#include <cstring>
#include <cstdint>

namespace Engine {
    //use a power of two size for BUFFER_SIZE!
    template<typename T, size_t BUFFER_SIZE>
    class circular_buffer {
        private:
            size_t m_ReadPos    = 0;
            size_t m_WritePos   = 0;
            size_t m_Size       = 0;
            T*     m_Buffer     = nullptr;

            inline size_t increment(size_t n) noexcept {
                return (n + 1) % BUFFER_SIZE;
            }
            void init_buffer(size_t size) {
                try {
                    m_Buffer = new T[size];
                } catch (...) {
                    delete[] m_Buffer;
                    throw;
                }
            }
        public:
            circular_buffer() {
                init_buffer(BUFFER_SIZE);
            }
            ~circular_buffer() {
                delete[] m_Buffer;
            }
            circular_buffer(const circular_buffer& other) {
                m_ReadPos = other.m_ReadPos;
                m_WritePos = other.m_WritePos;
                m_Size = other.m_Size;
                init_buffer(BUFFER_SIZE);
                std::memcpy(&m_Buffer[0], &other.m_Buffer[0], BUFFER_SIZE);
            }
            circular_buffer& operator=(const circular_buffer& other) {
                if (&other != this) {
                    m_ReadPos = other.m_ReadPos;
                    m_WritePos = other.m_WritePos;
                    m_Size = other.m_Size;
                    init_buffer(BUFFER_SIZE);
                    std::memcpy(&m_Buffer[0], &other.m_Buffer[0], BUFFER_SIZE);
                }
                return *this;
            }
            circular_buffer(circular_buffer&& other) noexcept {
                m_ReadPos   = std::exchange(other.m_ReadPos, 0);
                m_WritePos  = std::exchange(other.m_WritePos, 0);
                m_Size      = std::exchange(other.m_Size, 0);
                m_Buffer    = std::exchange(other.m_Buffer, nullptr);
            }
            circular_buffer& operator=(circular_buffer&& other) noexcept {
                if (&other != this) {
                    delete[] m_Buffer;
                    m_ReadPos  = std::exchange(other.m_ReadPos, 0);
                    m_WritePos = std::exchange(other.m_WritePos, 0);
                    m_Size     = std::exchange(other.m_Size, 0);
                    m_Buffer   = std::exchange(other.m_Buffer, nullptr);
                }
                return *this;
            }
            [[nodiscard]] inline bool empty() const noexcept { return m_Size == 0; }
            [[nodiscard]] inline size_t capacity() const noexcept { return BUFFER_SIZE; }
            [[nodiscard]] inline size_t size() const noexcept { return m_Size; }

            [[nodiscard]] inline T& getReadItem() noexcept { return m_Buffer[m_ReadPos]; }
            [[nodiscard]] inline T& getNextReadItem() noexcept { return m_Buffer[increment(m_ReadPos)]; }

            [[nodiscard]] inline size_t getReadIndex() noexcept { return m_ReadPos; }
            [[nodiscard]] inline size_t getNextReadIndex() noexcept { return increment(m_ReadPos); }
            [[nodiscard]] inline size_t getWriteIndex() noexcept { return m_WritePos; }
            [[nodiscard]] inline size_t getNextWriteIndex() noexcept { return increment(m_WritePos); }


            void clear() {
                m_ReadPos  = 0;
                m_WritePos = 0;
                m_Size     = 0;
            }

            bool push(const T& val) {
                const auto next_tail = increment(m_WritePos);
                m_Buffer[m_WritePos] = val;
                m_WritePos           = next_tail;
                m_Size               = std::min(BUFFER_SIZE, m_Size + 1);
                return true;
            }
            bool push(T&& val) {
                const auto next_tail = increment(m_WritePos);
                m_Buffer[m_WritePos] = std::move(val);
                m_WritePos           = next_tail;
                m_Size               = std::min(BUFFER_SIZE, m_Size + 1);
                return true;
            }
            template<class ... ARGS>
            bool emplace(ARGS&&... args) {
                const auto next_tail = increment(m_WritePos);
                m_Buffer[m_WritePos] = T{ std::forward<ARGS>(args)... };
                m_WritePos           = next_tail;
                m_Size               = std::min(BUFFER_SIZE, m_Size + 1);
                return true;
            }
            bool pop() {
                if (m_Size == 0) {
                    return false;
                }
                m_ReadPos = increment(m_ReadPos);
                --m_Size;
                return true;
            }
    };
}

#endif