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
            std::vector<std::string> result;
            if (str.empty())
                return result;
            std::stringstream ss(str);
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, ',');
                result.push_back(substr);
            }
            if (result.size() == 0)
                result.push_back(str);
            return result;
        }
        static float GetRandomFloatFromTo(const float& minValue, const float& maxValue) {
            const float result = minValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxValue - minValue)));
            return result;
        }
        static int GetRandomIntFromTo(const int& minVal, const int& maxVal) {
            if (minVal == maxVal)
                return minVal;
            const int result = minVal + static_cast<int>(rand()) / (static_cast<int>(RAND_MAX / (  (maxVal+1) - minVal)));
            return result;
        }
};


#endif