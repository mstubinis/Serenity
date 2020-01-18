#include "Security.h"

#include <bcrypt/BCrypt.hpp>

using namespace std;

const string Security::bcrypt(const string& input, const int& workload) {
    return BCrypt::generateHash(input, workload);
}