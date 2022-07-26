#pragma once
#ifndef ENGINE_ALGORITHMS_RADIX_SORT_H
#define ENGINE_ALGORITHMS_RADIX_SORT_H

#include <vector>
#include <array>
#include <atomic>
#include <serenity/threading/ThreadingModule.h>
#include <boost/sort/spreadsort/float_sort.hpp>

namespace Engine::priv::algorithms {
    class radix_sort final {
        public:
            template<template<class> class CONTAINER, class ... TYPE, class INT_TYPE>
            static constexpr void count_sort(CONTAINER<TYPE...>& input, std::vector<TYPE...>& aux, std::array<INT_TYPE, 10>& buckets, int exp, size_t begin, size_t end) noexcept {
                int i = 0;
                if constexpr (!std::is_same_v<INT_TYPE, std::atomic<int>>) {
                    buckets.fill(INT_TYPE{ 0 });
                }
                for (i = begin; i <= end; i++) {
                    int digit = (input[i] / exp) % 10;
                    buckets[digit]++;
                }
                for (i = 1; i < buckets.size(); i++) {
                    buckets[i] += buckets[i - 1];
                }
                for (i = end; i >= int(begin); i--) {
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
namespace Engine::algorithms {
    namespace detail {


#define PREFETCH 1
#if PREFETCH
        #include <xmmintrin.h>	// for prefetch
        inline constexpr void prefetch_64(const auto x, const auto idx) noexcept { _mm_prefetch(reinterpret_cast<const char*>(x + idx + 64), 0); }
        inline constexpr void prefetch_128(const auto x, const auto idx) noexcept { _mm_prefetch(reinterpret_cast<const char*>(x + idx + 128), 0); }
#else
        inline constexpr void prefetch_64(const auto x) noexcept {}
        inline constexpr void prefetch_128(const auto x) noexcept {}
#endif


        inline constexpr uint32_t FloatFlip(uint32_t f) noexcept {
            return f ^ (-int32_t(f >> 31) | 0b1000'0000'0000'0000'0000'0000'0000'0000);
        }
        // ================================================================================================
        // flip a float back (invert FloatFlip)
        //  if sign is 1 (negative), it flips the sign bit back
        //  if sign is 0 (positive), it flips all bits back
        // ================================================================================================
        inline constexpr uint32_t IFloatFlip(uint32_t f) noexcept {
            return f ^ (((f >> 31) - 1) | 0b1000'0000'0000'0000'0000'0000'0000'0000);
        }
        // ---- utils for accessing 11-bit quantities
        inline constexpr auto BIT_CONVERT_0(const auto x) noexcept { return x & 2047; }
        inline constexpr auto BIT_CONVERT_1(const auto x) noexcept { return x >> 11 & 2047; }
        inline constexpr auto BIT_CONVERT_2(const auto x) noexcept { return x >> 22; }

        template<uint32_t SIZE = 2048>
        struct histrogram {
            std::array<uint32_t, SIZE> b = { 0 };

            [[nodiscard]] inline uint32_t size() const noexcept { return SIZE; }
            inline void increment(const auto loc) noexcept {
                ++b[loc];
            }
            [[nodiscard]] inline uint32_t& operator[](const auto index) noexcept {
                return b[index];
            }
            [[nodiscard]] inline const uint32_t& operator[](const auto index) const noexcept {
                return b[index];
            }
            inline void reset() noexcept {
                b.fill(0);
            }
        };

    }

    template<size_t HIST_SIZE = 2048>
    class float_sort_parallel {
        private:
            std::vector<float> m_Buffer;

            std::vector<detail::histrogram<HIST_SIZE>> m_HistJobs0;
            std::vector<detail::histrogram<HIST_SIZE>> m_HistJobs1;
            std::vector<detail::histrogram<HIST_SIZE>> m_HistJobs2;

            detail::histrogram<HIST_SIZE> m_Hist0;
            detail::histrogram<HIST_SIZE> m_Hist1;
            detail::histrogram<HIST_SIZE> m_Hist2;

        public:
            float_sort_parallel() = default;
            float_sort_parallel(float* farr, size_t size, bool writeToSource = true) {
                sort(farr, size, writeToSource);
            }
            float_sort_parallel(auto& floatContainer, size_t size, bool writeToSource = true) {
                sort(floatContainer.data(), size, writeToSource);
            }
            float_sort_parallel(auto& floatContainer, bool writeToSource = true) {
                sort(floatContainer.data(), floatContainer.size(), writeToSource);
            }

            void sort(float* floatArray, size_t size, bool writeToSource = true) {
                const size_t numJobs = Engine::hardware_concurrency();
                m_Buffer.clear();
                m_Buffer.resize(size, 0.0f);

                uint32_t* sortedBuffer = reinterpret_cast<uint32_t*>(m_Buffer.data());
                uint32_t* inputBuffer  = reinterpret_cast<uint32_t*>(floatArray);

                m_HistJobs0.clear();
                m_HistJobs0.resize(numJobs);
                m_HistJobs1.clear();
                m_HistJobs1.resize(numJobs);
                m_HistJobs2.clear();
                m_HistJobs2.resize(numJobs);

                auto sync_main_histogram = [this, numJobs]() {
                    m_Hist0.reset();
                    m_Hist1.reset();
                    m_Hist2.reset();
                    for (size_t job = 0; job != numJobs; ++job) {
                        const size_t total = size_t(m_HistJobs0[job].size());
                        for (size_t i = 0; i != total; ++i) {
                            m_Hist0[i] += m_HistJobs0[job][i];
                            m_Hist1[i] += m_HistJobs1[job][i];
                            m_Hist2[i] += m_HistJobs2[job][i];
                        }
                    }
                };
                Engine::priv::threading::addJobSplitVectored([inputBuffer, this](size_t index, int32_t jobIndex) {
                    detail::prefetch_64(inputBuffer, index);
                    m_HistJobs0[jobIndex].increment(detail::BIT_CONVERT_0(detail::FloatFlip(inputBuffer[index])));
                }, false, false, size, Engine::hardware_concurrency() / 3);
                Engine::priv::threading::addJobSplitVectored([inputBuffer, this](size_t index, int32_t jobIndex) {
                    detail::prefetch_64(inputBuffer, index);
                    m_HistJobs1[jobIndex].increment(detail::BIT_CONVERT_1(detail::FloatFlip(inputBuffer[index])));
                }, false, false, size, Engine::hardware_concurrency() / 3);
                Engine::priv::threading::addJobSplitVectored([inputBuffer, this](size_t index, int32_t jobIndex) {
                    detail::prefetch_64(inputBuffer, index);
                    m_HistJobs2[jobIndex].increment(detail::BIT_CONVERT_2(detail::FloatFlip(inputBuffer[index])));
                }, false, true, size, Engine::hardware_concurrency() / 3);
                Engine::priv::threading::waitForAll();
                sync_main_histogram();

                // 2.  Sum the histograms -- each histogram entry records the number of values preceding itself.
                uint32_t sum0 = 0;
                uint32_t sum1 = 0;
                uint32_t sum2 = 0;
                uint32_t tsum;
                for (uint32_t index = 0; index != m_Hist0.size(); ++index) {
                    tsum = m_Hist0[index] + sum0;
                    m_Hist0[index] = sum0 - 1;
                    sum0 = tsum;

                    tsum = m_Hist1[index] + sum1;
                    m_Hist1[index] = sum1 - 1;
                    sum1 = tsum;

                    tsum = m_Hist2[index] + sum2;
                    m_Hist2[index] = sum2 - 1;
                    sum2 = tsum;
                }
                for (size_t index = 0; index != size; ++index) {
                    detail::prefetch_128(inputBuffer, index);
                    const uint32_t fi = detail::FloatFlip(inputBuffer[index]);
                    const auto i = detail::BIT_CONVERT_0(fi);
                    sortedBuffer[++m_Hist0[i]] = fi;
                }
                for (size_t index1 = 0; index1 != size; ++index1) {
                    detail::prefetch_128(sortedBuffer, index1);
                    const uint32_t si = sortedBuffer[index1];
                    const auto i = detail::BIT_CONVERT_1(si);
                    inputBuffer[++m_Hist1[i]] = si;
                }
                for (size_t index2 = 0; index2 != size; ++index2) {
                    detail::prefetch_128(inputBuffer, index2);
                    const uint32_t ai = inputBuffer[index2];
                    const auto i = detail::BIT_CONVERT_2(ai);
                    sortedBuffer[++m_Hist2[i]] = detail::IFloatFlip(ai);
                }
                if (writeToSource) {
                    std::memcpy(floatArray, m_Buffer.data(), size * sizeof(float));
                }
            }
    };



    template<template<class> class CONTAINER, class ... TYPE>
    void radix_sort(CONTAINER<TYPE...>& input) {
        using ELEMENT_TYPE      = std::tuple_element_t<0, std::tuple<TYPE...>>;
        if constexpr (std::is_floating_point_v<ELEMENT_TYPE>) {
            boost::sort::spreadsort::float_sort(std::begin(input), std::end(input));
        } else {
            ELEMENT_TYPE maxElement = *std::max_element(std::cbegin(input), std::cend(input));
            std::vector<ELEMENT_TYPE> aux(input.size(), 0);
            std::array<int, 10> buckets = { 0 };
            for (int exp = 1; maxElement / exp > 0; exp *= 10) {
                Engine::priv::algorithms::radix_sort::count_sort(input, aux, buckets, exp, 0, input.size() - 1);
            }
        }
    }
}


/*
    uint32_t* sort  = (uint32_t*)sorted;
    uint32_t* arr   = (uint32_t*)farray;

    histrogram_one<2048> hist0;
    histrogram_one<2048> hist1;
    histrogram_one<2048> hist2;

    for (uint32_t i = 0; i != size; ++i) {
        prefetch_64(arr, i);
        const uint32_t floatFlipped = FloatFlip(arr[i]);
        ++hist0[(BIT_CONVERT_0(floatFlipped))];
        ++hist1[(BIT_CONVERT_1(floatFlipped))];
        ++hist2[(BIT_CONVERT_2(floatFlipped))];
    }
    uint32_t sum0 = 0;
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;
    uint32_t tsum;
    for (uint32_t i = 0; i != hist0.size(); ++i) {
        tsum = hist0[i] + sum0;
        hist0[i] = sum0 - 1;
        sum0 = tsum;

        tsum = hist1[i] + sum1;
        hist1[i] = sum1 - 1;
        sum1 = tsum;

        tsum = hist2[i] + sum2;
        hist2[i] = sum2 - 1;
        sum2 = tsum;
    }
    for (uint32_t i = 0; i != size; ++i) {
        prefetch_128(arr, i);
        const auto floatFlipped = FloatFlip(arr[i]);
        sort[++hist0[BIT_CONVERT_0(floatFlipped)]] = floatFlipped;
    }
    for (uint32_t i = 0; i != size; ++i) {
        prefetch_128(sort, i);
        const auto floatAsUint = sort[i];
        arr[++hist1[BIT_CONVERT_1(floatAsUint)]] = floatAsUint;
    }
    for (uint32_t i = 0; i != size; ++i) {
        prefetch_128(arr, i);
        const auto floatAsUint = arr[i];
        sort[++hist2[BIT_CONVERT_2(floatAsUint)]] = IFloatFlip(floatAsUint);
    }
*/

#endif