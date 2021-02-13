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
            std::atomic<uint32_t> m_ReadPos  = 0;
            std::atomic<uint32_t> m_WritePos = 0;
            T*                    m_Buffer   = nullptr;

            inline uint32_t increment(uint32_t n) noexcept {
                return (n + 1) % BUFFER_SIZE;
            }
            void init_buffer(size_t size) {
                try {
                    m_Buffer = new T[size];
                }catch (...) {
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
                m_ReadPos.store(other.m_ReadPos.load());
                m_WritePos.store(other.m_WritePos.load());
                init_buffer(BUFFER_SIZE);
                std::memcpy(&m_Buffer[0], &other.m_Buffer[0], BUFFER_SIZE);
            }
            circular_buffer& operator=(const circular_buffer& other) {
                if (&other != this) {
                    m_ReadPos.store(other.m_ReadPos.load());
                    m_WritePos.store(other.m_WritePos.load());
                    init_buffer(BUFFER_SIZE);
                    std::memcpy(&m_Buffer[0], &other.m_Buffer[0], BUFFER_SIZE);
                }
                return *this;
            }
            circular_buffer(circular_buffer&& other) noexcept {
                other.m_ReadPos.store(m_ReadPos.load());
                other.m_WritePos.store(m_WritePos.load());
                m_Buffer       = other.m_Buffer;
                other.m_Buffer = nullptr;
            }
            circular_buffer& operator=(circular_buffer&& other) noexcept {
                if (&other != this) {
                    other.m_ReadPos.store(m_ReadPos.load());
                    other.m_WritePos.store(m_WritePos.load());
                    m_Buffer       = other.m_Buffer;
                    other.m_Buffer = nullptr;
                }
                return *this;
            }

            bool try_push(const T& val) {
                const auto current_tail = m_WritePos.load();
                const auto next_tail    = increment(current_tail);
                if (next_tail != m_ReadPos.load()){
                    m_Buffer[current_tail] = val;
                    m_WritePos.store(next_tail);
                    return true;
                }
                return false;
            }
            void push(const T& val) {
                bool result = false;
                do {
                    result = try_push(val);
                } while (!result);
            }
            bool try_push(T&& val) {
                const auto current_tail = m_WritePos.load();
                const auto next_tail    = increment(current_tail);
                if (next_tail != m_ReadPos.load()) {
                    m_Buffer[current_tail] = std::move(val);
                    m_WritePos.store(next_tail);
                    return true;
                }
                return false;
            }
            void push(T&& val) {
                bool result = false;
                do {
                    result = try_push(val);
                } while (!result);
            }


            bool try_pop(T* pval){
                auto currentHead = m_ReadPos.load();
                if (currentHead == m_WritePos.load()){
                    return false;
                }
                *pval = m_Buffer[currentHead];
                m_ReadPos.store(increment(currentHead));
                return true;
            }
            T pop() {
                T    ret;
                bool result = false;
                do {
                    result = try_pop(&ret);
                } while (!result);
                return ret;
            }

    };
}

#endif