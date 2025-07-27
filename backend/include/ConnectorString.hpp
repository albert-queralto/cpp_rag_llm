#ifndef CONNECTIONSTRING_HPP
#define CONNECTIONSTRING_HPP

#include <string>
#include <unordered_map>
#include <stdexcept>

class ConnectionString {
private:
    std::unordered_map<std::string, std::string> connection;

public:
    ConnectionString() {
        connection = {
            {"postgresql", "postgresql://"},
            {"mysql", "mysql://"},
            {"oracle", "oracle://"},
            {"mssql", "mssql://"},
            {"sqlite", "sqlite://"}
        };
    }

    std::string operator()(const std::string& connection_type,
                            const std::string& username,
                            const std::string& password,
                            const std::string& host,
                            const std::string& database_name = "",
                            const std::string& port = "") const {
        if (connection.find(connection_type) == connection.end()) {
            throw std::runtime_error("Unsupported connection type: " + connection_type);
        }

        std::string connection_string = connection.at(connection_type);

        if (connection_type == "sqlite") {
            return connection_string;
        }

        connection_string += username + ":" + password + "@" + host;

        if (!port.empty()) {
            connection_string += ":" + port;
        }

        if (!database_name.empty()) {
            connection_string += "/" + database_name;
        }

        return connection_string;
    }
};

#endif // CONNECTIONSTRING_HPP