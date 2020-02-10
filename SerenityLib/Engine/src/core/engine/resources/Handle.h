#pragma once
#ifndef ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD
#define ENGINE_RESOURCE_HANDLE_INCLUDE_GUARD

#include <core/engine/utils/Utils.h>

class  EngineResource;

struct Handle final {
    std::uint32_t index   : 12;
    std::uint32_t version : 15;
    std::uint32_t type    : 5;

    explicit Handle();
    explicit Handle(const std::uint32_t index_, const std::uint32_t version_, const std::uint32_t type_);
    operator std::uint32_t() const;
    const bool null() const;
    const EngineResource* get() const;
    const EngineResource* operator ->() const;
};

#endif