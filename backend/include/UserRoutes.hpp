#ifndef USERROUTES_HPP
#define USERROUTES_HPP

#include <crow_all.h>
#include <sqlite3.h>
#include "DatabaseConnector.hpp"
#include "Authentication.hpp"

class UserRoutes {
public:
    static crow::response sign_user_up(const crow::request& req);
    static crow::response sign_user_in(const crow::request& req);
    static crow::response get_all_users(const crow::request& req);
    static crow::response get_user_by_email(const crow::request& req, const std::string& email);
    static crow::response update_user(const crow::request& req, const std::string& email);
    static crow::response delete_user_by_email(const crow::request& req, const std::string& email);
    static crow::response logout(const crow::request& req);
};

#endif // USERROUTES_HPP