#pragma once
#ifndef ENGINE_ALGORITHMS_MERGE_SORT_H
#define ENGINE_ALGORITHMS_MERGE_SORT_H

#include <serenity/core/engine/threading/ThreadingModule.h>

namespace Engine::priv::algorithms {
    class MergeSort final {
        public:
            template<template<class> class CONTAINER, class ... TYPE>
            static constexpr void merge(CONTAINER<TYPE...>& input, std::vector<TYPE...>& aux, const size_t left, const size_t middle, const size_t right) noexcept {
                auto i = left;
                auto j = middle + 1;
                auto k = left;
                while (i <= middle && j <= right) {
                    if (input[i] <= input[j]) {
                        aux[k++] = input[i++];
                    }else {
                        aux[k++] = input[j++];
                    }
                }
                while (i <= middle) {
                    aux[k++] = input[i++];
                }
                while (j <= right) {
                    aux[k++] = input[j++];
                }
                for (i = left; i < k; ++i) {
                    input[i] = aux[i];
                }
            }
            template<template<class> class CONTAINER, class ... TYPE>
            static constexpr void divide(CONTAINER<TYPE...>& input, std::vector<TYPE...>& aux, const size_t left, const size_t right) noexcept {
                if (left < right) {
                    const auto middle = left + (right - left) / 2;
                    Engine::priv::algorithms::MergeSort::divide(input, aux, left,         middle);
                    Engine::priv::algorithms::MergeSort::divide(input, aux, middle + 1,   right);
                    Engine::priv::algorithms::MergeSort::merge(input,  aux, left, middle, right);
                }
            }
            template<typename ITERATOR>
            static constexpr void mergesort_mt(ITERATOR begin, ITERATOR end, uint32_t N = std::thread::hardware_concurrency() / 2) {
                auto len = std::distance(begin, end);
                if (len <= 1024 || N < 2){
                    std::sort(begin, end);
                    return;
                }
                ITERATOR mid = std::next(begin, len / 2);
                auto job = [begin, mid, N]() {
                    mergesort_mt(begin, mid, N - 2);
                };
                auto& fut = Engine::priv::threading::addJob(job);
                mergesort_mt(mid, end, N - 2);
                fut.wait();
                std::inplace_merge(begin, mid, end);
            }

    };
}
namespace Engine::algorithms {
    template<template<class> class CONTAINER, class ... TYPE>
    inline constexpr void merge_sort(CONTAINER<TYPE...>& input) {
        std::vector<TYPE...> aux(input.size(), 0);
        Engine::priv::algorithms::MergeSort::divide(input, aux, 0, input.size() - 1);
    }
    template<template<class> class CONTAINER, class ... TYPE>
    inline constexpr void merge_sort_multithreaded(CONTAINER<TYPE...>& input) noexcept {
        Engine::priv::algorithms::MergeSort::mergesort_mt(input.begin(), input.end());
    }
}

#endif