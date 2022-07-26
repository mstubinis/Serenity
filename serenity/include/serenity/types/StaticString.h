#pragma once
#ifndef ENGINE_TYPES_STATIC_STRING_H
#define ENGINE_TYPES_STATIC_STRING_H

#include <string>

namespace Engine {
    class static_string {
        private:
            const char* m_Data{ nullptr };
            size_t      m_Size{ 0 };
        public:
            constexpr static_string() noexcept = default;

            constexpr static_string(const char* data, size_t size) noexcept
                : m_Data{ data }
                , m_Size{ size }
            {}

            template<size_t N> constexpr static_string(const char (&data)[N]) noexcept
                : m_Data{ &data[0] }
                , m_Size{ N - 1 }
            {}

            inline constexpr size_t size() const noexcept { return m_Size; }
            inline constexpr const char* c_str() const noexcept { return m_Data; }

            inline explicit operator std::string() const { return std::string{ m_Data }; }
    };
}

#endif