#include "Helper.h"

#include <sstream>
#include <cstdlib>
#include <random>

using namespace std;

string Helper::FormatTimeAsMinThenSecs(const unsigned int& mins, const unsigned int& secs) {
    string res;
    if (mins < 10) {
        res = "0" + to_string(mins);
    }else {
        res = to_string(mins);
    }
    res += ":";
    if (secs < 10) {
        res += "0" + to_string(secs);
    }else {
        res += to_string(secs);
    }
    return res;
}
vector<string> Helper::SeparateStringByCharacter(const string& str, const char character_seperator) {
    vector<string> result;
    if (str.empty())
        return result;
    stringstream ss(str);
    while (ss.good()) {
        string substr;
        getline(ss, substr, character_seperator);
        result.push_back(substr);
    }
    if (result.size() == 0)
        result.push_back(str);
    return result;
}
unordered_set<string> Helper::SeparateStringByCharacterIntoSet(const string& str, const char character_seperator) {
    unordered_set<string> result;
    if (str.empty())
        return result;
    stringstream ss(str);
    while (ss.good()) {
        string substr;
        getline(ss, substr, character_seperator);
        result.insert(substr);
    }
    if (result.size() == 0)
        result.insert(str);
    return result;
}
string Helper::Stringify(vector<string>& list, const char character_seperator) {
    if (list.size() == 0) return "";
    if (list.size() == 1) return list[0];
    string res = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        res += character_seperator + list[i];
    }
    return res;
}
string Helper::Stringify(unordered_set<string>& list, const char character_seperator) {
    if (list.size() == 0) return "";
    if (list.size() == 1) return list.begin()._Ptr->_Myval;
    string res = list.begin()._Ptr->_Myval;
    unsigned int count = 0;
    for (auto& itr : list) {
        if (count > 0) {
            res += character_seperator + itr;
        }
        ++count;
    }
    return res;
}
float Helper::GetRandomFloatFromTo(const float& minValue, const float& maxValue) {
    const float result = minValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxValue - minValue)));
    return result;
}
int Helper::GetRandomIntFromTo(const int& minValue, const int& maxValue) {
    if (minValue == maxValue)
        return minValue;
    random_device device;
    mt19937 mt(device());
    uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(mt);
}