#include <crow_all.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <ctime>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "Authentication.hpp"
#include "UserRoutes.hpp"
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

// Custom middleware for session handling
struct SessionMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        res.add_header("Set-Cookie", "session_id=example_session_id; HttpOnly");
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        // No post-processing needed
    }
};

void ensure_admin_user() {
    try {
        std::string email = "admin@admin.com";
        std::string password = "admin";
        std::string role = "admin";

        // Generate the combined salt and hashed password
        std::string combined = HashPassword::create(password);

        // Get the current time in the desired format
        auto now = std::time(nullptr);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        std::string formatted_time = oss.str();

        // Connect to the database
        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);

        // Check if the admin user already exists
        std::string query_check = "SELECT email FROM users WHERE email = ?";
        sqlite3_stmt* stmt_check;
        if (sqlite3_prepare_v2(db, query_check.c_str(), -1, &stmt_check, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare query: " + std::string(sqlite3_errmsg(db)));
        }
        sqlite3_bind_text(stmt_check, 1, email.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt_check) == SQLITE_ROW) {
            std::cout << "[INFO] Admin user already exists: " << email << std::endl;
            sqlite3_finalize(stmt_check);
            sqlite3_close(db);
            return;
        }
        sqlite3_finalize(stmt_check);

        // Insert the admin user into the database
        std::string query_insert = "INSERT INTO users (email, password, role, created_date, update_date) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt_insert;
        if (sqlite3_prepare_v2(db, query_insert.c_str(), -1, &stmt_insert, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare insert query: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_bind_text(stmt_insert, 1, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 2, combined.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 3, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 4, formatted_time.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 5, formatted_time.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt_insert) != SQLITE_DONE) {
            throw std::runtime_error("Failed to execute insert query: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_finalize(stmt_insert);
        sqlite3_close(db);
        std::cout << "[INFO] Admin user created successfully: " << email << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
    }
}

int main() {
    crow::App<SessionMiddleware> app; // Add custom middleware

    // Ensure the admin user exists
    ensure_admin_user();

    // Set up logging
    crow::logger::setLogLevel(crow::LogLevel::Info);

    // Define routes
    /* ------------------- User Routes ----------------------*/
    CROW_ROUTE(app, "/api/users").methods("GET"_method)([](const crow::request& req) {
        UserRoutes userRoutes;
        return userRoutes.get_all_users(req);
    });
    CROW_ROUTE(app, "/api/signup").methods("POST"_method)([](const crow::request& req) {
        UserRoutes userRoutes;
        return userRoutes.sign_user_up(req);
    });
    CROW_ROUTE(app, "/api/users/<string>").methods("GET"_method)([](const crow::request& req, const std::string& email) {
        UserRoutes userRoutes;
        return userRoutes.get_user_by_email(req, email);
    });
    CROW_ROUTE(app, "/api/users/<string>").methods("PUT"_method)([](const crow::request& req, const std::string& email) {
        UserRoutes userRoutes;
        return userRoutes.update_user(req, email);
    });
    CROW_ROUTE(app, "/api/users/<string>").methods("DELETE"_method)([](const crow::request& req, const std::string& email) {
        UserRoutes userRoutes;
        return userRoutes.delete_user_by_email(req, email);
    });

    /* ------------------- Authentication Routes ----------------------*/
    CROW_ROUTE(app, "/api/signin").methods("POST"_method)([](const crow::request& req) {
        UserRoutes userRoutes;
        return userRoutes.sign_user_in(req);
    });
    CROW_ROUTE(app, "/api/logout").methods("POST"_method)([](const crow::request& req) {
        UserRoutes userRoutes;
        return userRoutes.logout(req);
    });

    CROW_ROUTE(app, "/api/preprocessing")([](const crow::request& req) {
        return crow::response(200, "Preprocessing route");
    });

    CROW_ROUTE(app, "/api/inference")([](const crow::request& req) {
        return crow::response(200, "Inference route");
    });

    CROW_ROUTE(app, "/api/doc_collections")([](const crow::request& req) {
        return crow::response(200, "Doc collections route");
    });

    // Start the server
    app.port(4000).multithreaded().run();

    return 0;
}