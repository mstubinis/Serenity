#pragma once
#ifndef ENGINE_TYPES_FLAG_H
#define ENGINE_TYPES_FLAG_H

namespace Engine {
    template<typename T> 
    class Flag final {
        private:
            T m_Flags = (T)0;
        public:
            Flag() = default;
            Flag(T other)
                : m_Flags{ other }
            {}

            inline T operator&(T other) const noexcept { return m_Flags & other; }
            inline T operator|(T other) const noexcept { return m_Flags | other; }
            inline T operator&=(T other) const noexcept { return m_Flags &= other; }
            inline T operator|=(T other) const noexcept { return m_Flags |= other; }

            inline constexpr T get() const noexcept { return m_Flags; }
            inline constexpr bool has(T flag) const noexcept { return (m_Flags & flag); }
            inline void add(T flag) noexcept { m_Flags |= flag; }
            inline void remove(T flag) noexcept { m_Flags &= ~flag; }
        };
};

#endif