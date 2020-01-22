#include "Security.h"

#include <argon2.h>
#include <vector>
#include <random>

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/sha.h>


using namespace std;

constexpr auto MAX_LENGTH = 13;

const string Security::sha256(const string& input) {
    CryptoPP::byte abDigest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(abDigest, (CryptoPP::byte*)input.data(), input.size());
    return string((char*)abDigest);
}

const unsigned int Security::string_to_int_hash(const string& input) {
    unsigned long res = 0;
    for (int i = 0; i < input.length(); i++)
        res += pow(27, MAX_LENGTH - i - 1) * (1 + input[i] - 'a');
    return static_cast<unsigned int>(res);
}

const string Security::encrypt_aes(const string& input, const unsigned int& nonce_, const unsigned int& in_key) {
    //Key and IV setup
    //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
    //bit). This key is secretly exchanged between two parties before communication   
    //begins. DEFAULT_KEYLENGTH= 16 bytes
    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte nonce[CryptoPP::AES::BLOCKSIZE];

    memset(key, in_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    memset(nonce, nonce_, CryptoPP::AES::BLOCKSIZE);

    string ciphertext;

    // Create Cipher Text
    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, nonce);

    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, NEW CryptoPP::StringSink(ciphertext));
    stfEncryptor.Put(reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    stfEncryptor.MessageEnd();

    return ciphertext;

}
const string Security::decrypt_aes(const string& input, const unsigned int& nonce_, const unsigned int& in_key) {
    //Key and IV setup
    //AES encryption uses a secret key of a variable length (128-bit, 196-bit or 256-   
    //bit). This key is secretly exchanged between two parties before communication   
    //begins. DEFAULT_KEYLENGTH= 16 bytes
    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    CryptoPP::byte nonce[CryptoPP::AES::BLOCKSIZE];

    memset(key, in_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    memset(nonce, nonce_, CryptoPP::AES::BLOCKSIZE);

    string decryptedtext;

    // Decrypt
    CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, nonce);

    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, NEW CryptoPP::StringSink(decryptedtext));
    stfDecryptor.Put(reinterpret_cast<const unsigned char*>(input.c_str()), input.size());
    stfDecryptor.MessageEnd();

    return decryptedtext;
}


const string Security::argon2id(const string& salt, const string& input_password, const int& parallelism, const uint32_t& iterations, const uint32_t& memory_cost, const uint32_t& salt_len, const uint32_t& hash_len) {
    vector<uint8_t> hash;
    hash.resize(hash_len, 0);

    uint8_t* pwd = (uint8_t*)_strdup(input_password.c_str());
    uint32_t pwdlen = (uint32_t)strlen((char*)pwd);

    vector<uint8_t> salt_array;
    salt_array.reserve(salt_len);
    for (int i = 0; i < salt_len; ++i) {
        salt_array.push_back(static_cast<uint8_t>(salt[i]));
    }

    const auto process_res = argon2id_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, salt_array.data(), salt_len, (void*)hash.data(), hash_len);
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
    uint32_t pwdlen = (uint32_t)strlen((char*)pwd);

    vector<uint8_t> salt_array;
    salt_array.reserve(salt_len);
    for (int i = 0; i < salt_len; ++i) {
        salt_array.push_back(static_cast<uint8_t>(salt[i]));
    }

    const auto process_res = argon2i_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, salt_array.data(), salt_len, (void*)hash.data(), hash_len);
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
    uint32_t pwdlen = (uint32_t)strlen((char*)pwd);

    vector<uint8_t> salt_array;
    salt_array.reserve(salt_len);
    for (int i = 0; i < salt_len; ++i) {
        salt_array.push_back(static_cast<uint8_t>(salt[i]));
    }

    const auto process_res = argon2d_hash_raw(iterations, memory_cost, parallelism, pwd, pwdlen, salt_array.data(), salt_len, (void*)hash.data(), hash_len);
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
    std::mt19937 mt(device() + static_cast<unsigned int>(time(0)) - user);
    for (size_t i = 0; i < salt_len; ++i) {
        const char c = static_cast<char>(distribution_1(mt));
        res += c;
    }
    return res;
}