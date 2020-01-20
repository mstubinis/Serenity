#include "Security.h"

#include <argon2.h>
#include <vector>
#include <random>

using namespace std;

const string Security::argon2id(const string& salt, const string& input_password, const int& parallelism, const uint32_t& iterations, const uint32_t& memory_cost, const uint32_t& salt_len, const uint32_t& hash_len) {
    vector<uint8_t> hash;
    hash.resize(hash_len, 0);

    uint8_t* pwd = (uint8_t*)_strdup(input_password.c_str());
    uint32_t pwdlen = strlen((char*)pwd);

    const auto process_res = argon2id_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, (void*)salt.data(), salt_len, (void*)hash.data(), hash_len);
    string res;
    for (size_t i = 0; i < hash_len; ++i) {
        res += static_cast<char>(hash[i]);
    }
    return res;
}
const string Security::argon2i(const string& salt, const string& input_password, const int& parallelism, const uint32_t& iterations, const uint32_t& memory_cost, const uint32_t& salt_len, const uint32_t& hash_len) {
    vector<uint8_t> hash;
    hash.resize(hash_len, 0);

    uint8_t* pwd = (uint8_t*)_strdup(input_password.c_str());
    uint32_t pwdlen = strlen((char*)pwd);

    const auto process_res = argon2i_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, (void*)salt.data(), salt_len, (void*)hash.data(), hash_len);
    string res;
    for (size_t i = 0; i < hash_len; ++i) {
        res += static_cast<char>(hash[i]);
    }
    return res;
}
const string Security::argon2d(const string& salt, const string& input_password, const int& parallelism, const uint32_t& iterations, const uint32_t& memory_cost, const uint32_t& salt_len, const uint32_t& hash_len) {
    vector<uint8_t> hash;
    hash.resize(hash_len, 0);

    uint8_t* pwd = (uint8_t*)_strdup(input_password.c_str());
    uint32_t pwdlen = strlen((char*)pwd);

    const auto process_res = argon2d_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, (void*)salt.data(), salt_len, (void*)hash.data(), hash_len);
    string res = "";
    for (size_t i = 0; i < hash_len; ++i) {
        res += static_cast<char>(hash[i]);
    }
    return res;
}




const string Security::generate_user_salt(const string& username, const uint32_t& salt_len) {
    string res = "";
    std::random_device device;
    std::mt19937 mt_1(device());
    uniform_int_distribution<int> distribution_1(0, 256);
    uniform_int_distribution<int> distribution(-128, 127);
    unsigned int user = 0;
    for (auto& character : username) {
        user += (character + static_cast<char>(distribution(mt_1))) / 2;
    }
    std::mt19937 mt(device() * time(0) * user);
    for (size_t i = 0; i < salt_len; ++i) {
        const char c = static_cast<char>(distribution_1(mt));
        res += c;
    }
    return res;
}