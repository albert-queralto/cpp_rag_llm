#include "Authentication.hpp"
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// Function to generate a unique salt
std::string generate_salt(size_t length) {
    unsigned char salt[length];
    if (RAND_bytes(salt, length) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }
    return std::string(reinterpret_cast<char*>(salt), length);
}

// Implementation of JWTHandler methods
std::string JWTHandler::create_access_token(const std::string& email, const std::string& role) {
    auto token = jwt::create()
        .set_payload_claim("email", jwt::claim(email))
        .set_payload_claim("role", jwt::claim(role))
        .sign(jwt::algorithm::hs256{"your_secret_key"});
    return token;
}

nlohmann::json JWTHandler::verify_access_token(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"your_secret_key"})
            .with_claim("email", jwt::claim(std::string()))
            .with_claim("role", jwt::claim(std::string()));

        verifier.verify(decoded);

        nlohmann::json data;
        data["email"] = decoded.get_payload_claim("email").as_string();
        data["role"] = decoded.get_payload_claim("role").as_string();
        return data;
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid access token: " + std::string(e.what()));
    }
}

// Implementation of HashPassword methods
std::string base64_encode(const unsigned char* input, size_t length) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    bio = BIO_push(bio, bmem);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BUF_MEM* bptr;
    BIO_get_mem_ptr(bio, &bptr);
    std::string output(bptr->data, bptr->length);
    BIO_free_all(bio);
    return output;
}

std::string HashPassword::create(const std::string& password) {
    const size_t hash_len = 32;
    const size_t salt_len = 16;
    unsigned char hash[hash_len];
    unsigned char salt[salt_len];

    // Generate a random salt
    if (RAND_bytes(salt, salt_len) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    // Hash the password with the salt
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, salt_len, 10000, EVP_sha256(), hash_len, hash) == 0) {
        throw std::runtime_error("Failed to hash password");
    }

    // Encode salt and hash into Base64
    std::string encoded_salt = base64_encode(salt, salt_len);
    std::string encoded_hash = base64_encode(hash, hash_len);

    // Combine encoded salt and hash into a single string
    std::string combined = encoded_salt + ":" + encoded_hash;

    // Print the salt and hashed password for debugging
    std::cout << "[DEBUG] Generated salt (Base64): " << encoded_salt << std::endl;
    std::cout << "[DEBUG] Generated hashed password (Base64): " << encoded_hash << std::endl;

    return combined;
}

std::string base64_decode(const std::string& input) {
    BIO* bio = BIO_new_mem_buf(input.data(), input.size());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    char buffer[input.size()];
    int decoded_length = BIO_read(bio, buffer, input.size());
    BIO_free_all(bio);
    return std::string(buffer, decoded_length);
}

bool HashPassword::verify(const std::string& password, const std::string& combined) {
    const size_t hash_len = 32;
    const size_t salt_len = 16;

    // Split the combined string into salt and hash
    size_t delimiter_pos = combined.find(':');
    if (delimiter_pos == std::string::npos) {
        throw std::runtime_error("Invalid combined string format");
    }
    std::string encoded_salt = combined.substr(0, delimiter_pos);
    std::string encoded_hash = combined.substr(delimiter_pos + 1);

    // Decode the salt and hash from Base64
    std::string salt = base64_decode(encoded_salt);
    std::string stored_hash = base64_decode(encoded_hash);

    unsigned char hash[hash_len];

    // Hash the provided password with the extracted salt
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), reinterpret_cast<const unsigned char*>(salt.c_str()), salt_len, 10000, EVP_sha256(), hash_len, hash) == 0) {
        throw std::runtime_error("Failed to hash password for verification");
    }

    // Compare the newly generated hash with the stored hash
    return stored_hash == std::string(reinterpret_cast<char*>(hash), hash_len);
}