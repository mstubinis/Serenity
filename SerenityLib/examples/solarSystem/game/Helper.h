#pragma once
#ifndef GAME_HELPER_FUNCTIONS_H
#define GAME_HELPER_FUNCTIONS_H

#include <vector>
#include <string>
#include <unordered_set>

class Helper final {
    public:
        static std::string FormatTimeAsMinThenSecs(const unsigned int& mins, const unsigned int& secs);
        static std::vector<std::string> SeparateStringByCharacter(const std::string& str, const char character_seperator);
        static std::unordered_set<std::string> SeparateStringByCharacterIntoSet(const std::string& str, const char character_seperator);
        static std::string Stringify(std::vector<std::string>& list, const char character_seperator);
        static std::string Stringify(std::unordered_set<std::string>& list, const char character_seperator);
        static float GetRandomFloatFromTo(const float& minValue, const float& maxValue);
        static int GetRandomIntFromTo(const int& minVal, const int& maxVal);
};


#endif