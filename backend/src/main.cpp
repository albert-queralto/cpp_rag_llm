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
#include "ChromaClient.hpp"
#include <filesystem>
#include <vector>
#include <set>
#include "DocumentChain.hpp"


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

// Utility function to save uploaded file
bool save_uploaded_file(const std::string& dir, const std::string& filename, const std::string& file_content) {
    std::filesystem::create_directories(dir);
    std::ofstream ofs(dir + "/" + filename, std::ios::binary);
    if (!ofs) return false;
    ofs.write(file_content.data(), file_content.size());
    return ofs.good();
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

    /* ------------------- ChromaDB Routes ----------------------*/
    ChromaClient chromaClient("http", "chromadb", "8000");

    // Endpoint to get ChromaDB version
    CROW_ROUTE(app, "/api/chroma/version").methods("GET"_method)([&chromaClient](const crow::request&) {
        return crow::response(200, chromaClient.get_version());
    });

    // Endpoint to get ChromaDB heartbeat
    CROW_ROUTE(app, "/api/chroma/heartbeat").methods("GET"_method)([&chromaClient](const crow::request&) {
        return crow::response(200, chromaClient.get_heartbeat());
    });

    // Endpoint to get all collections
    CROW_ROUTE(app, "/api/collections").methods("GET"_method)([&chromaClient](const crow::request&) {
        try {
            auto collections = chromaClient.get_collections();
            json response = json::array();
            for (const auto& collection : collections) {
                response.push_back({
                    {"name", collection.GetName()},
                    {"id", collection.GetId()}
                });
            }
            return crow::response(200, response.dump());
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to fetch collections: " << e.what() << std::endl;
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    // Endpoint to create a collection
    CROW_ROUTE(app, "/api/collections/<string>").methods("POST"_method)([&chromaClient](const crow::request&, const std::string& name) {
        try {
            chromadb::Collection collection = chromaClient.create_collection(name);
            json response = {{"collection_id", collection.GetId()}};
            return crow::response(201, response.dump());
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    // Endpoint to update a collection
    CROW_ROUTE(app, "/api/collections/<string>").methods("PUT"_method)([&chromaClient](const crow::request& req, const std::string& name) {
        try {
            // Parse the request body
            auto body = json::parse(req.body);

            // Validate the "name" field
            if (!body.contains("name") || !body["name"].is_string()) {
                throw std::invalid_argument("Invalid or missing 'name' field in request payload");
            }

            std::string newName = body["name"].get<std::string>();

            // Fetch the collection by name
            chromadb::Collection collection = chromaClient.get_collection(name);

            // Update the collection name with an empty metadata map
            std::unordered_map<std::string, std::string> emptyMetadata;
            chromadb::Collection updatedCollection = chromaClient.update_collection(name, newName, emptyMetadata);

            // Prepare the response
            json response = {{"collection_name", updatedCollection.GetName()}};
            return crow::response(200, response.dump());
        } catch (const chromadb::ChromaNotFoundException& e) {
            std::cerr << "[ERROR] Collection not found: " << e.what() << std::endl;
            return crow::response(404, std::string("Error: Collection not found"));
        } catch (const std::invalid_argument& e) {
            std::cerr << "[ERROR] Invalid request payload: " << e.what() << std::endl;
            return crow::response(400, std::string("Error: ") + e.what());
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to update collection: " << e.what() << std::endl;
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    // Endpoint to delete a collection
    CROW_ROUTE(app, "/api/collections/<string>").methods("DELETE"_method)([&chromaClient](const crow::request&, const std::string& name) {
        try {
            // Fetch the collection by name
            chromadb::Collection collection = chromaClient.get_collection(name);

            // Delete the collection
            bool success = chromaClient.delete_collection(collection);

            // Prepare the response
            json response = {{"collection_name", name}, {"status", success ? "deleted" : "failed"}};
            return crow::response(200, response.dump());
        } catch (const chromadb::ChromaNotFoundException& e) {
            std::cerr << "[ERROR] Collection not found: " << e.what() << std::endl;
            return crow::response(404, std::string("Error: Collection not found"));
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to delete collection: " << e.what() << std::endl;
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    // Endpoint to add embeddings to a collection
    CROW_ROUTE(app, "/api/collections/<string>/embeddings").methods("POST"_method)([&chromaClient](const crow::request& req, const std::string& name) {
        try {
            auto body = json::parse(req.body);
            std::vector<std::string> ids = body["ids"].get<std::vector<std::string>>();
            std::vector<std::vector<double>> embeddings = body["embeddings"].get<std::vector<std::vector<double>>>();
            std::vector<std::unordered_map<std::string, std::string>> metadatas = body["metadatas"].get<std::vector<std::unordered_map<std::string, std::string>>>();

            chromadb::Collection collection = chromaClient.create_collection(name);
            chromaClient.add_embeddings(collection, ids, embeddings, metadatas);

            return crow::response(200, "Embeddings added successfully");
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    // Endpoint to query a collection
    CROW_ROUTE(app, "/api/collections/<string>/query").methods("POST"_method)([&chromaClient](const crow::request& req, const std::string& name) {
        try {
            auto body = json::parse(req.body);
            std::vector<std::string> ids = body["ids"].get<std::vector<std::string>>();
            std::vector<std::vector<double>> embeddings = body["embeddings"].get<std::vector<std::vector<double>>>();
            int limit = body["limit"].get<int>();

            chromadb::Collection collection = chromaClient.create_collection(name);
            auto queryResponse = chromaClient.query(collection, ids, embeddings, limit);

            json response = json::array();
            for (const auto& resource : queryResponse) {
                json metadatas_json = json::array();
                if (resource.metadatas) {
                    for (const auto& metadata : *resource.metadatas) {
                        metadatas_json.push_back(metadata);
                    }
                }

                json resourceJson = {
                    {"ids", resource.ids},
                    {"metadatas", metadatas_json}
                };
                response.push_back(resourceJson);
            }

            return crow::response(200, response.dump());
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Error: ") + e.what());
        }
    });

    /* ------------------- Adding and Preprocessing documents ----------------------*/
    CROW_ROUTE(app, "/api/upload/<string>").methods("POST"_method)
    ([](const crow::request& req, const std::string& collection_name) {
        // Expecting: header "X-Filename" with the filename, and the file content in req.body
        auto filename_it = req.headers.find("X-Filename");
        if (filename_it == req.headers.end()) {
            return crow::response(400, "Missing X-Filename header");
        }
        std::string filename = filename_it->second;
        std::string dir_path = "data/files/" + collection_name;
        if (!save_uploaded_file(dir_path, filename, req.body)) {
            return crow::response(500, "Failed to save file");
        }

        // Call document chain logic
        std::string file_path = dir_path + "/" + filename;
        std::string result = process_document_chain(file_path, collection_name);

        nlohmann::json resp = {{"message", result}};
        return crow::response(200, resp.dump());
    });

    CROW_ROUTE(app, "/api/get_document_filenames/<string>")
    ([](const crow::request& req, const std::string& collection_name) {
        try {
            ChromaClient chromaClient("http", "chromadb", "8000");
            auto collection = chromaClient.get_collection(collection_name);

            // Prepare include vector to get metadatas
            std::vector<std::string> ids; // empty means all
            std::vector<std::string> include = {"metadatas"};
            nlohmann::json where_document;
            nlohmann::json where;

            // Get all embeddings/resources
            auto resources = chromaClient.get_embeddings(collection, ids, include, where_document, where);

            std::vector<std::string> filenames;
            for (const auto& resource : resources) {
                if (resource.metadata && resource.metadata->count("source")) {
                    std::string source = resource.metadata->at("source");
                    auto pos = source.find_last_of("/\\");
                    filenames.push_back(pos != std::string::npos ? source.substr(pos + 1) : source);
                }
            }

            nlohmann::json resp = {{"filenames", filenames}};
            return crow::response(200, resp.dump());
        } catch (const std::exception& e) {
            return crow::response(404, "Collection or documents not found.");
        }
    });



    /* ------------------- Other Routes ----------------------*/

    CROW_ROUTE(app, "/api/inference")([](const crow::request& req) {
        return crow::response(200, "Inference route");
    });

    // Start the server
    app.port(4000).multithreaded().run();

    return 0;
}