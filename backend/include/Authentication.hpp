#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>

// Function declarations for salt generation
std::string generate_salt(size_t length);

// Class declaration for JWT handling
class JWTHandler {
public:
    static std::string create_access_token(const std::string& email, const std::string& role);
    static nlohmann::json verify_access_token(const std::string& token);
};

// Class declaration for password hashing and verification
class HashPassword {
public:
    static std::string create(const std::string& password);
    static bool verify(const std::string& password, const std::string& hashed_password);
};

#endif // AUTHENTICATION_HPP