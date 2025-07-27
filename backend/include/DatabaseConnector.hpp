#ifndef DATABASECONNECTOR_HPP
#define DATABASECONNECTOR_HPP

#include "Connector.hpp"
#include "ConnectorString.hpp"
#include <iostream>
#include <stdexcept>
#include <sqlite3.h>
#include <pqxx/pqxx>
#include <variant>

// Define a variant type for database connections
using DatabaseConnection = std::variant<sqlite3*, pqxx::connection*>;

class DatabaseConnector : public Connector {
private:
    std::string connection_type;
    std::string username;
    std::string password;
    std::string host;
    std::string database_name;
    std::string port;

public:
    DatabaseConnector(const std::string& connection_type,
                        const std::string& username,
                        const std::string& password,
                        const std::string& host,
                        const std::string& database_name = "",
                        const std::string& port = "")
        : connection_type(connection_type),
            username(username),
            password(password),
            host(host),
            database_name(database_name),
            port(port) {}

    DatabaseConnection connect() override {
        ConnectionString connection_string_generator;
        try {
            std::string connection_string = connection_string_generator(
                connection_type, username, password, host, database_name, port);

            std::cout << "Connecting to database with connection string: " << connection_string << std::endl;

            if (connection_type == "sqlite") {
                sqlite3* db;
                if (sqlite3_open(database_name.c_str(), &db) != SQLITE_OK) {
                    throw std::runtime_error("Failed to connect to SQLite database");
                }
                std::cout << "Successfully connected to SQLite database!" << std::endl;
                return db;
            } else if (connection_type == "postgresql") {
                pqxx::connection* conn = new pqxx::connection(connection_string);
                if (conn->is_open()) {
                    std::cout << "Successfully connected to PostgreSQL database!" << std::endl;
                    return conn;
                } else {
                    delete conn;
                    throw std::runtime_error("Failed to open the PostgreSQL database connection.");
                }
            } else {
                throw std::runtime_error("Unsupported connection type: " + connection_type);
            }
        } catch (const std::exception& e) {
            throw std::runtime_error("Error connecting to database: " + std::string(e.what()));
        }
    }
};

#endif // DATABASECONNECTOR_HPP