#pragma once
#ifndef ENGINE_THREADING_HELPERS_H
#define ENGINE_THREADING_HELPERS_H

#include <vector>
#include <thread>
#include <serenity/system/TypeDefs.h>

namespace Engine {
    [[nodiscard]] inline uint32_t hardware_concurrency() noexcept {
        return std::max(1U, std::thread::hardware_concurrency());
        //return 1U;
    }

    //creates a vector of pairs, each pair contains a start and ending index to iterate over a very large single vector
    [[nodiscard]] std::vector<std::pair<size_t, size_t>> splitVectorPairs(size_t vectorSize) noexcept;
}

#endif