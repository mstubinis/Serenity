#pragma once
#ifndef GAME_HELPER_FUNCTIONS_H
#define GAME_HELPER_FUNCTIONS_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

class Helper {
    public:
        static std::vector<std::string> SeparateStringByCharacter(const std::string& str, const char character) {
            std::stringstream ss(str);
            std::vector<std::string> result;
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, ',');
                result.push_back(substr);
            }
            if (result.size() == 0)
                result.push_back(str);
            return result;
        }
        static float GetRandomFloatFromTo(const unsigned int& minValue, const unsigned int& maxValue) {
            const float result = minValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxValue - minValue)));
            return result;
        }
};

#endif