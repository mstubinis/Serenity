#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_H
#define ENGINE_RESOURCE_HANDLE_H

#include <core/engine/resources/Resource.h>
#include <core/engine/utils/Utils.h>

class SoundQueue;
class Handle final {
    friend class ::SoundQueue;
    private:
        Resource* get_base() const noexcept;

        std::uint32_t m_Index : 12;
        std::uint32_t m_Version : 15;
        std::uint32_t m_Type : 5;
    public:
        explicit Handle() {
            m_Index   = 0U;
            m_Version = 0U;
            m_Type    = 0U;
        }
        explicit Handle(std::uint32_t index_, std::uint32_t version_, std::uint32_t type_) {
            m_Index   = index_;
            m_Version = version_;
            m_Type    = type_;
        }
        constexpr inline std::uint32_t index() const noexcept {
            return m_Index;
        }
        constexpr inline std::uint32_t version() const noexcept {
            return m_Version;
        }
        constexpr inline std::uint32_t type() const noexcept {
            return m_Type;
        }
        constexpr inline operator std::uint32_t() const noexcept {
            return m_Type << 27 | m_Version << 12 | m_Index;
        }
        constexpr inline bool null() const noexcept {
            return (m_Type == ResourceType::Empty || m_Index == 0U);
        }
        template<typename RESOURCE> inline RESOURCE* get() const noexcept {
            Resource* resource = Handle::get_base();
            return reinterpret_cast<RESOURCE*>(resource);
            //return static_cast<T*>(resource);
        }
};

#endif