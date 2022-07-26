
#include <serenity/threading/ThreadingHelpers.h>

std::vector<std::pair<size_t, size_t>> Engine::splitVectorPairs(size_t vectorSize) noexcept {
    std::vector<std::pair<size_t, size_t>> outVec;
    if (vectorSize > 0) {
        const auto num_cores = Engine::hardware_concurrency();
        const auto capacity = std::min(num_cores, uint32_t(vectorSize));
        outVec.reserve(capacity);
        if (vectorSize <= num_cores) {
            outVec.emplace_back(0, vectorSize - 1);
            return outVec;
        }
        size_t c = vectorSize / num_cores;
        size_t remainder = vectorSize % num_cores; /* Likely uses the result of the division. */
        size_t accumulator = 0;
        //std::pair<size_t, size_t> res;
        size_t b;
        size_t e = (num_cores - remainder);
        for (size_t i = 0; i < capacity; ++i) {
            if (c == 0) {
                b = remainder - 1;
            } else {
                b = accumulator + (c - 1);
            }
            if (i == e) {
                if (i != (num_cores - remainder)) {
                    ++accumulator;
                    ++b;
                }
                ++b;
                ++e;
            }
            outVec.emplace_back(accumulator, b);
            accumulator += c;
        }
    }
    return outVec;
}