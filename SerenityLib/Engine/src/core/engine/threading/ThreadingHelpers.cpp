#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadingHelpers.h>

std::vector<std::pair<size_t, size_t>> Engine::splitVectorPairs(size_t vectorSize, size_t num_cores) noexcept {
    if (num_cores == 0) {
        num_cores = Engine::hardware_concurrency();
    }
    std::vector<std::pair<size_t, size_t>> outVec;
    if (vectorSize <= num_cores) {
        outVec.emplace_back(0, vectorSize - 1);
        return outVec;
    }
    outVec.reserve(num_cores);

    size_t c = vectorSize / num_cores;
    size_t remainder = vectorSize % num_cores; /* Likely uses the result of the division. */
    size_t accumulator = 0;
    std::pair<size_t, size_t> res;
    size_t b;
    size_t e = (num_cores - remainder);
    for (size_t i = 0; i < std::min(num_cores, vectorSize); ++i) {
        if (c == 0)
            b = remainder - 1;
        else
            b = accumulator + (c - 1);
        if (i == e) {
            if (i != (num_cores - remainder)) {
                ++accumulator;
                ++b;
            }
            ++b;
            ++e;
        }
        res = outVec.emplace_back(accumulator, b);
        accumulator += c;
    }
    return outVec;
}