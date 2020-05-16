#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD
#define ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/utils/Utils.h>

class Handle final {
    private:
        EngineResource* get_base() const;
    public:
        std::uint32_t index   : 12;
        std::uint32_t version : 15;
        std::uint32_t type    : 5;

        explicit Handle();
        explicit Handle(const std::uint32_t index_, const std::uint32_t version_, const std::uint32_t type_);
        operator std::uint32_t() const;
        const bool null() const;


        template<typename T> T* get() const {
            EngineResource* resource = get_base();
            return reinterpret_cast<T*>(resource);
            //return static_cast<T*>(resource);
        }
};

#endif