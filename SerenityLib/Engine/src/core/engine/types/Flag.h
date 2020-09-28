#pragma once
#ifndef ENGINE_TYPES_FLAG_H
#define ENGINE_TYPES_FLAG_H

namespace Engine {
    template<typename T> 
    class Flag final {
        private:
            T m_Flags = (T)0;
        public:
            explicit Flag() = default;
            ~Flag() = default;
            explicit Flag(const T& other)
                : m_Flags{ other }
            {}
            explicit Flag(T&& other) 
                : m_Flags{ std::move(other) }
            {}

            Flag& operator=(const T& other) {
                m_Flags = other;
                return *this;
            }
            Flag& operator=(T&& other) {
                m_Flags = std::move(other);
                return *this;
            }
            inline CONSTEXPR const T& get() const noexcept { return m_Flags; }
            inline CONSTEXPR bool has(const T& flag) const noexcept { return (m_Flags & flag); }
            inline T operator&(const T& other) const noexcept { return m_Flags & other; }
            inline T operator|(const T& other) const noexcept { return m_Flags | other; }
            inline T operator&=(const T& other) const noexcept { return m_Flags &= other; }
            inline T operator|=(const T& other) const noexcept { return m_Flags |= other; }

            void add(const T& flag) {
                m_Flags = m_Flags | flag;
            }
            void remove(const T& flag) {
                m_Flags = m_Flags & ~flag;
            }
            void add(T&& flag) {
                m_Flags = m_Flags | flag;
            }
            void remove(T&& flag) {
                m_Flags = m_Flags & ~flag;
            }
        };
};

#endif