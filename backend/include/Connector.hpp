#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <variant>
#include <sqlite3.h>
#include <pqxx/pqxx>

// Define a variant type for database connections
using DatabaseConnection = std::variant<sqlite3*, pqxx::connection*>;

// Abstract base class for connectors
class Connector {
public:
    virtual ~Connector() = default;
    virtual DatabaseConnection connect() = 0;
};

#endif // CONNECTOR_HPP