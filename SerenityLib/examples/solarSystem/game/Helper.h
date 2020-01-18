#pragma once
#ifndef GAME_HELPER_FUNCTIONS_H
#define GAME_HELPER_FUNCTIONS_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <random>
#include <unordered_set>

class Helper final {
    public:
        static std::default_random_engine GENERATOR;

        static std::vector<std::string> SeparateStringByCharacter(const std::string& str, const char character_seperator) {
            std::vector<std::string> result;
            if (str.empty())
                return result;
            std::stringstream ss(str);
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, character_seperator);
                result.push_back(substr);
            }
            if (result.size() == 0)
                result.push_back(str);
            return result;
        }
        static std::unordered_set<std::string> SeparateStringByCharacterIntoSet(const std::string& str, const char character_seperator) {
            std::unordered_set<std::string> result;
            if (str.empty())
                return result;
            std::stringstream ss(str);
            while (ss.good()) {
                std::string substr;
                std::getline(ss, substr, character_seperator);
                result.insert(substr);
            }
            if (result.size() == 0)
                result.insert(str);
            return result;
        }
        static std::string Stringify(std::vector<std::string>& list, const char character_seperator) {
            if (list.size() == 0) return "";
            if (list.size() == 1) return list[0];
            std::string res = list[0];
            for (size_t i = 1; i < list.size(); ++i) {
                res += character_seperator + list[i];
            }
            return res;
        }
        static std::string Stringify(std::unordered_set<std::string>& list, const char character_seperator) {
            if (list.size() == 0) return "";
            if (list.size() == 1) return list.begin()._Ptr->_Myval;
            std::string res = list.begin()._Ptr->_Myval;
            unsigned int count = 0;
            for (auto& itr : list) {
                if (count > 0) {
                    res += character_seperator + itr;
                }
                ++count;
            }
            return res;
        }

        static float GetRandomFloatFromTo(const float& minValue, const float& maxValue) {
            const float result = minValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxValue - minValue)));
            return result;
        }
        static int GetRandomIntFromTo(const int& minVal, const int& maxVal) {
            if (minVal == maxVal)
                return minVal;
            std::uniform_int_distribution<int> distribution(minVal, maxVal);
            return distribution(GENERATOR);
        }
};


#endif