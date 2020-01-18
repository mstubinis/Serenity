#pragma once
#ifndef GAME_SECURITY_H
#define GAME_SECURITY_H

#include <string>

class Security final {
    private:

    public:
        static inline const std::string bcrypt(const std::string& input, const int& workload = 12);
};

#endif