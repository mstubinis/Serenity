#pragma once
#ifndef ENGINE_THREADING_HELPERS_H
#define ENGINE_THREADING_HELPERS_H

namespace Engine {
    unsigned int hardware_concurrency();

    //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
    template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& v, size_t num_cores = 0U) {
        if (num_cores == 0)
            num_cores = Engine::hardware_concurrency();
        const auto vs = v.size();
        std::vector<std::vector<T>> outVec;
        auto length   = vs / num_cores;
        auto remain   = vs % num_cores;
        size_t begin  = 0;
        size_t end    = 0;
        for (auto i = 0; i < std::min(num_cores, vs); ++i) {
            end += (remain > 0) ? (length + !!(remain--)) : length;
            outVec.emplace_back(v.begin() + begin, v.begin() + end);
            begin = end;
        }
        return outVec;
    }
    //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
    template<typename T> std::vector<std::vector<unsigned int>> splitVectorIndices(const std::vector<T>& v, size_t num_cores = 0U) {
        if (num_cores == 0)
            num_cores = Engine::hardware_concurrency();
        const auto vs = v.size();
        std::vector<std::vector<unsigned int>> outVec;
        auto length   = vs / num_cores;
        auto remain   = vs % num_cores;
        size_t begin  = 0;
        size_t end    = 0;
        auto splitAmount = std::min(num_cores, vs);
        for (auto i = 0; i < splitAmount; ++i) {
            outVec.emplace_back();
            end += (remain > 0) ? (length + !!(remain--)) : length;
            auto splitSize = end - begin;
            outVec[i].resize(splitSize);
            for (auto j = 0; j < splitSize; ++j) {
                outVec[i][j] = static_cast<unsigned int>(begin + j);
            }
            begin = end;
        }
        return outVec;
    }
    //creates a vector of pairs, each pair contains a start and ending index to iterate over a very large single vector

    std::vector<std::pair<size_t, size_t>> splitVectorPairs(size_t vectorSize, size_t num_cores);

    template<typename T> std::vector<std::pair<size_t, size_t>> splitVectorPairs(const std::vector<T>& v, size_t num_cores = 0U) {
        return Engine::splitVectorPairs(static_cast<size_t>(v.size()), num_cores);
    }
}

#endif