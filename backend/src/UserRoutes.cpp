#include "UserRoutes.hpp"
#include <nlohmann/json.hpp>
#include <ctime>
#include <stdexcept>
#include <variant>
#include <iostream>

using json = nlohmann::json;

crow::response UserRoutes::sign_user_up(const crow::request& req) {
    try {
        std::cout << "Parsing request body..." << std::endl;
        auto body = json::parse(req.body);
        std::string email = body["email"];
        std::string password = body["password"];
        std::string role = body.contains("role") ? body["role"] : "user";
        std::cout << "Request body parsed successfully. Email: " << email << ", Role: " << role << std::endl;

        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);
        std::cout << "Connected to database successfully." << std::endl;

        // Check if the user already exists
        std::string query_check = "SELECT email FROM users WHERE email = ?";
        sqlite3_stmt* stmt_check;
        if (sqlite3_prepare_v2(db, query_check.c_str(), -1, &stmt_check, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return crow::response(500, "Database query preparation error");
        }
        sqlite3_bind_text(stmt_check, 1, email.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt_check) == SQLITE_ROW) {
            std::cout << "User already exists: " << email << std::endl;
            sqlite3_finalize(stmt_check);
            sqlite3_close(db);
            json response = {{"error", "User already exists"}};
            return crow::response(409, response.dump());
        }
        sqlite3_finalize(stmt_check);

        // Hash the password
        std::cout << "Hashing password..." << std::endl;
        std::string hashed_password = HashPassword::create(password);
        std::cout << "Password hashed successfully." << std::endl;

        // Insert the new user
        std::string query_insert = "INSERT INTO users (email, password, role, created_date, update_date) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt_insert;
        if (sqlite3_prepare_v2(db, query_insert.c_str(), -1, &stmt_insert, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare insert query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return crow::response(500, "Database insert query preparation error");
        }
        sqlite3_bind_text(stmt_insert, 1, email.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 2, hashed_password.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 3, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 4, std::to_string(std::time(nullptr)).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt_insert, 5, std::to_string(std::time(nullptr)).c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt_insert) != SQLITE_DONE) {
            std::cerr << "Failed to execute insert query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt_insert);
            sqlite3_close(db);
            json response = {{"error", "Failed to create user"}};
            return crow::response(500, response.dump());
        }

        sqlite3_finalize(stmt_insert);
        sqlite3_close(db);
        json response = {{"message", "User created successfully"}};
        std::cout << "User created successfully: " << email << std::endl;
        return crow::response(201, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        json response = {{"error", std::string("Internal Server Error: ") + e.what()}};
        return crow::response(500, response.dump());
    }
}

crow::response UserRoutes::sign_user_in(const crow::request& req) {
    try {
        std::cout << "[DEBUG] Parsing request body..." << std::endl;
        auto body = json::parse(req.body);
        std::string email = body["email"];
        std::string password = body["password"];
        std::cout << "[DEBUG] Request body parsed successfully. Email: " << email << std::endl;

        // Connect to the database
        std::cout << "[DEBUG] Connecting to the database..." << std::endl;
        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);
        std::cout << "[DEBUG] Connected to the database successfully." << std::endl;

        // Prepare the query to fetch user details
        std::string query = "SELECT password, role FROM users WHERE email = ?";
        sqlite3_stmt* stmt;
        std::cout << "[DEBUG] Preparing query: " << query << std::endl;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[ERROR] Failed to prepare query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            json response = {{"error", "Database query preparation error"}};
            return crow::response(500, response.dump());
        }

        // Bind the email parameter
        std::cout << "[DEBUG] Binding email parameter: " << email << std::endl;
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);

        // Execute the query
        std::cout << "[DEBUG] Executing query..." << std::endl;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string stored_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::cout << "[DEBUG] Query executed successfully. Role: " << role << std::endl;

            sqlite3_finalize(stmt);
            sqlite3_close(db);

            // Verify the password
            std::cout << "[DEBUG] Verifying password..." << std::endl;
            if (HashPassword::verify(password, stored_password)) {
                std::cout << "[DEBUG] Password verified successfully." << std::endl;

                // Generate an access token
                std::string access_token = JWTHandler::create_access_token(email, role);
                json response = {
                    {"email", email},
                    {"role", role},
                    {"token", {
                        {"access_token", access_token},
                        {"token_type", "bearer"}
                    }}
                };
                std::cout << "[DEBUG] Access token generated successfully." << std::endl;
                return crow::response(200, response.dump());
            } else {
                std::cerr << "[ERROR] Invalid credentials for email: " << email << std::endl;
                json response = {{"error", "Invalid credentials"}};
                return crow::response(401, response.dump());
            }
        }

        // User does not exist
        std::cerr << "[ERROR] User does not exist: " << email << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        json response = {{"error", "User does not exist"}};
        return crow::response(404, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception occurred: " << e.what() << std::endl;
        json response = {{"error", std::string("Internal Server Error: ") + e.what()}};
        return crow::response(500, response.dump());
    }
}

crow::response UserRoutes::get_all_users(const crow::request& req) {
    try {
        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);

        if (!db) {
            std::cerr << "Failed to connect to the database." << std::endl;
            return crow::response(500, "Database connection error");
        }

        std::string query = "SELECT email, role, created_date, update_date FROM users";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

        json users = json::array();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            json user = {
                {"email", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
                {"role", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
                {"created_date", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))},
                {"update_date", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))}
            };
            users.push_back(user);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        json response = {{"users", users}};
        std::cout << "Response JSON: " << response.dump() << std::endl;

        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return crow::response(500, std::string("Internal Server Error: ") + e.what());
    }
}

crow::response UserRoutes::get_user_by_email(const crow::request& req, const std::string& email) {
    try {
        std::cout << "[DEBUG] Fetching user details for email: " << email << std::endl;

        // Connect to the database
        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);

        // Prepare the query
        std::string query = "SELECT email, role, created_date, update_date FROM users WHERE email = ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[ERROR] Failed to prepare query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return crow::response(500, "Database query preparation error");
        }

        // Bind the email parameter
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);

        // Execute the query
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string fetched_email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string created_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string update_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            sqlite3_finalize(stmt);
            sqlite3_close(db);

            // Return user details as JSON
            json response = {
                {"email", fetched_email},
                {"role", role},
                {"created_date", created_date},
                {"update_date", update_date}
            };
            return crow::response(200, response.dump());
        }

        // User not found
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        json response = {{"error", "User not found"}};
        return crow::response(404, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception occurred: " << e.what() << std::endl;
        json response = {{"error", std::string("Internal Server Error: ") + e.what()}};
        return crow::response(500, response.dump());
    }
}

crow::response UserRoutes::update_user(const crow::request& req, const std::string& email) {
    try {
        std::cout << "[DEBUG] Parsing request body..." << std::endl;
        auto body = json::parse(req.body);
        std::string password = body["password"];
        std::string role = body["role"];
        std::string created_date = body["created_date"];
        std::string update_date = body["update_date"];
        std::cout << "[DEBUG] Request body parsed successfully. Email: " << email << ", Role: " << role << std::endl;

        // Hash the password
        std::cout << "[DEBUG] Hashing password..." << std::endl;
        std::string hashed_password = HashPassword::create(password);
        std::cout << "[DEBUG] Password hashed successfully. Hashed Password: " << hashed_password << std::endl;

        // Connect to the database
        std::cout << "[DEBUG] Connecting to the database..." << std::endl;
        DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
        auto connection = db_connector.connect();
        sqlite3* db = std::get<sqlite3*>(connection);
        std::cout << "[DEBUG] Connected to the database successfully." << std::endl;

        // Prepare the query
        std::string query = "UPDATE users SET password = ?, role = ?, created_date = ?, update_date = ? WHERE email = ?";
        sqlite3_stmt* stmt;
        std::cout << "[DEBUG] Preparing query: " << query << std::endl;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "[ERROR] Failed to prepare update query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return crow::response(500, "Database update query preparation error");
        }

        // Bind parameters
        std::cout << "[DEBUG] Binding parameters..." << std::endl;
        sqlite3_bind_text(stmt, 1, hashed_password.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, role.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, created_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, update_date.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, email.c_str(), -1, SQLITE_STATIC);
        std::cout << "[DEBUG] Parameters bound successfully." << std::endl;

        // Execute the query
        std::cout << "[DEBUG] Executing query..." << std::endl;
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "[ERROR] Failed to execute update query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            json response = {{"error", "Failed to update user"}};
            return crow::response(500, response.dump());
        }
        std::cout << "[DEBUG] Query executed successfully." << std::endl;

        // Finalize and close the database connection
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        std::cout << "[DEBUG] Database connection closed successfully." << std::endl;

        // Return success response
        json response = {{"message", "User updated successfully"}};
        std::cout << "[DEBUG] User updated successfully: " << email << std::endl;
        return crow::response(200, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception occurred: " << e.what() << std::endl;
        json response = {{"error", std::string("Internal Server Error: ") + e.what()}};
        return crow::response(500, response.dump());
    }
}

crow::response UserRoutes::delete_user_by_email(const crow::request& req, const std::string& email) {
    DatabaseConnector db_connector("sqlite", "", "", "", "/root/home/data/database.db", "");
    auto connection = db_connector.connect(); // Get the DatabaseConnection variant
    sqlite3* db = std::get<sqlite3*>(connection); // Extract sqlite3* from the variant

    std::string query = "DELETE FROM users WHERE email = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return crow::response(404, "User does not exist");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return crow::response(200, "User deleted successfully");
}

crow::response UserRoutes::logout(const crow::request& req) {
    return crow::response(200, "Logged out successfully");
}