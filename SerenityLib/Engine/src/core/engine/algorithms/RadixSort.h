#pragma once
#ifndef ENGINE_ALGORITHMS_RADIX_SORT_H
#define ENGINE_ALGORITHMS_RADIX_SORT_H

namespace Engine::priv::algorithms {
    class radix_sort final {
        public:
            template<template<class> class CONTAINER, class ... TYPE, class INT_TYPE>
            static constexpr void count_sort(CONTAINER<TYPE...>& input, std::vector<TYPE...>& aux, std::array<INT_TYPE, 10>& buckets, int exp, size_t begin, size_t end) noexcept {
                int i = { 0 };
                if constexpr (!std::is_same<INT_TYPE, std::atomic<int>>::value) {
                    buckets.fill(INT_TYPE{ 0 });
                }
                for (i = begin; i <= end; i++) {
                    int digit = (input[i] / exp) % 10;
                    buckets[digit]++;
                }
                for (i = 1; i < buckets.size(); i++) {
                    buckets[i] += buckets[i - 1];
                }
                for (i = end; i >= (int)begin; i--) {
                    int digit = (input[i] / exp) % 10;
                    aux[buckets[digit] - 1] = input[i];
                    buckets[digit]--;
                }
                for (i = begin; i <= end; i++) {
                    input[i] = aux[i];
                }
            }
    };
}

#include <boost/sort/spreadsort/float_sort.hpp>
#include <core/engine/threading/ThreadingModule.h>

namespace Engine::algorithms {
    template<template<class> class CONTAINER, class ... TYPE>
    constexpr void radix_sort(CONTAINER<TYPE...>& input) {
        using ELEMENT_TYPE      = std::tuple_element_t<0, std::tuple<TYPE...>>;
        ELEMENT_TYPE maxElement = *std::max_element(input.cbegin(), input.cend());
        std::vector<ELEMENT_TYPE> aux(input.size(), 0);
        std::array<int, 10> buckets{ 0 };
        for (int exp = 1; maxElement / exp > 0; exp *= 10) {
            Engine::priv::algorithms::radix_sort::count_sort(input, aux, buckets, exp, 0, input.size() - 1);
        }
    }
    /*
    template<class EXECUTION_POLICY, template<class> class CONTAINER, class ... TYPE>
    constexpr void radix_sort(EXECUTION_POLICY&& policy, CONTAINER<TYPE...>& input) {
        if (std::is_same<EXECUTION_POLICY, std::execution::sequenced_policy>::value) {
            radix_sort(input);
        }else{
        }
    }
    */
    template<template<class> class CONTAINER, class ... TYPE>
    inline constexpr void radix_sort_floats(CONTAINER<TYPE...>& input) {
        boost::sort::spreadsort::float_sort(input.begin(), input.end());
    }
    /*
    template<class EXECUTION_POLICY, template<class> class CONTAINER, class ... TYPE>
    inline constexpr void radix_sort_floats(EXECUTION_POLICY&& policy, CONTAINER<TYPE...>& input) {
        if (std::is_same<EXECUTION_POLICY, std::execution::sequenced_policy>::value) {
            radix_sort_floats(input);
        }else{
            auto pairs = Engine::splitVectorPairs(input);
            for (size_t k = 0U; k < pairs.size(); ++k) {
                auto pair = pairs[k];
                Engine::priv::threading::addJob([&input, pair]() {
                    boost::sort::spreadsort::float_sort(input.begin() + pair.first, input.begin() + pair.second);
                });
            }
            Engine::priv::threading::waitForAll();
        }
    }
    */
}

#endif