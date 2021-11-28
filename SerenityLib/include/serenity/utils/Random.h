#pragma once
#ifndef ENGINE_UTILS_RANDOM_H
#define ENGINE_UTILS_RANDOM_H

#include <random>
#include <vector>

template<class T>
concept NUMBER = std::is_arithmetic_v<T>;

namespace Engine::priv {
    class RandomImpl {
        private:
            static std::mt19937 m_MT;
        public:
            template<NUMBER T>
            static T getRandomNumberImpl(T startInclusive, T endInclusive) {
                if constexpr( std::numeric_limits<T>::is_integer ) {
                    std::uniform_int_distribution<T> generatorInt(startInclusive, endInclusive);
                    return generatorInt(m_MT);
                } else {
                    std::uniform_real_distribution<T> generatorReal(startInclusive, endInclusive);
                    return generatorReal(m_MT);
                }
            }

            template<NUMBER T>
            static std::vector<T> getRandomNumbersImpl(T startInclusive, T endInclusive, size_t amountOfNumbers) {
                std::vector<T> numbers;
                numbers.reserve(amountOfNumbers);
                auto build_numbers = []<typename GENERATOR, NUMBER T>(T startInclusive, T endInclusive, size_t amountOfNumbers, std::vector<T>& numbers, std::mt19937& mt) {
                    GENERATOR generator(startInclusive, endInclusive);
                    for (size_t i = 0; i < amountOfNumbers; ++i) {
                        numbers.emplace_back(generator(mt));
                    }
                };
                if constexpr (std::numeric_limits<T>::is_integer) {
                    build_numbers.template operator()<std::uniform_int_distribution<T>, T>(startInclusive, endInclusive, amountOfNumbers, numbers, m_MT);
                } else {
                    build_numbers.template operator()<std::uniform_real_distribution<T>, T>(startInclusive, endInclusive, amountOfNumbers, numbers, m_MT);
                }
                return numbers;
            }
    };
}

namespace Engine::random {
    template<NUMBER T>
    T getRandomNumber(T startInclusive, T endInclusive) {
        return Engine::priv::RandomImpl::getRandomNumberImpl(startInclusive, endInclusive);
    }
    template<NUMBER T>
    std::vector<T> getRandomNumbers(T startInclusive, T endInclusive, size_t amountOfNumbers) {
        return Engine::priv::RandomImpl::getRandomNumbersImpl(startInclusive, endInclusive, amountOfNumbers);
    }
}

#endif