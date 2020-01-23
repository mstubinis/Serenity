#pragma once
#ifndef GAME_SECURITY_H
#define GAME_SECURITY_H

#include <string>

class Security final {
    private:

    public:
        static const std::string sha256(const std::string& input);

        static const unsigned int string_to_int_hash(const std::string& input);

        static const std::string encrypt_aes(const std::string& input, const unsigned int& nonce, const unsigned int& key);
        static const std::string decrypt_aes(const std::string& input, const unsigned int& nonce, const unsigned int& key);

        static const std::string generate_user_salt(
            const std::string& username, 
            const std::uint32_t& salt_len = 16
        );

        static const std::string argon2id(
            const std::string& salt,
            const std::string& input_password,
            const int& parallelism = 1,
            const std::uint32_t& iterations = 2,
            const std::uint32_t& memory_cost = (1 << 16),
            const std::uint32_t& salt_len = 16,
            const std::uint32_t& hash_len = 32
        );


        static const std::string argon2i(
            const std::string& salt,
            const std::string& input_password,
            const int& parallelism = 1,
            const std::uint32_t & iterations = 2,
            const std::uint32_t & memory_cost = (1 << 16),
            const std::uint32_t & salt_len = 16,
            const std::uint32_t & hash_len = 32
        );

        static const std::string argon2d(
            const std::string& salt,
            const std::string& input_password,
            const int& parallelism = 1,
            const std::uint32_t & iterations = 2,
            const std::uint32_t & memory_cost = (1 << 16),
            const std::uint32_t & salt_len = 16,
            const std::uint32_t & hash_len = 32
        );
};

#endif