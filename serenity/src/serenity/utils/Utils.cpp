#include <serenity/utils/Utils.h>

std::string numToCommasImpl(std::string s, int start, int end) noexcept {
    s = s.substr(start, end - start);
    int p = (int)s.length() - 3;
    while (p > 0) {
        s.insert(p, ",");
        p -= 3;
    }
    return s;
}