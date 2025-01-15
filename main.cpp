#include "crow.h"
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

int main() {
    crow::SimpleApp app;

    // Open SQLite database
    sqlite3* db;
    int rc = sqlite3_open("todo.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // Create table if it doesn't exist
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS todos (id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, completed INTEGER NOT NULL DEFAULT 0);", nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Health Check Route
    CROW_ROUTE(app, "/health")
    .methods("GET"_method)([]() {
        crow::json::wvalue health_status;
        health_status["status"] = "ok"; // You can also add more checks here if necessary
        return crow::response{200, health_status};
    });

    // Routes for todo app
    CROW_ROUTE(app, "/todos")
    .methods("GET"_method)([&db]() {
        std::vector<crow::json::wvalue> todos;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT id, title, completed FROM todos", -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue todo;
            todo["id"] = sqlite3_column_int(stmt, 0);
            todo["title"] = (const char*)sqlite3_column_text(stmt, 1);
            todo["completed"] = sqlite3_column_int(stmt, 2);
            todos.push_back(todo);
        }
        sqlite3_finalize(stmt);
        return crow::json::wvalue(todos);
    });

    CROW_ROUTE(app, "/add_todo").methods("POST"_method)([&db](const crow::request& req, crow::response& res) {
        auto json = crow::json::load(req.body);
        if (!json)
            res.code = 400, res.body = "{\"error\": \"Invalid JSON\"}";

        if(!json.has("title")){
            res.code = 400;
            res.body = "{\"error\": \"Title is required\"}";
            return;
        }

        std::string title = json["title"].s();

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "INSERT INTO todos (title, completed) VALUES (?, 0)", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error inserting todo: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            res.code = 500;
            res.body = "{\"error\": \"Failed to insert todo\"}";
            return;
        }

        sqlite3_finalize(stmt);
        res.code = 200;
    });

    CROW_ROUTE(app, "/update_todo/<int>").methods("PUT"_method)([&db](const crow::request& req, crow::response& res, int id) { 
        auto json = crow::json::load(req.body);
        if (!json) {
            res.code = 400;
            res.body = "{\"error\": \"Invalid JSON\"}";
            return;
        }

        if (!json.has("completed")) {
            res.code = 400;
            res.body = "{\"error\": \"Completed status is required\"}";
            return;
        }

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "UPDATE todos SET completed = ? WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, json["completed"].i());
        sqlite3_bind_int(stmt, 2, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error updating todo: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            res.code = 500;
            res.body = "{\"error\": \"Failed to update todo\"}";
            return;
        }

        sqlite3_finalize(stmt);
        res.code = 200;
    });

    CROW_ROUTE(app, "/delete_todo/<int>").methods("DELETE"_method)([&db](crow::response& res, int id) { 
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "DELETE FROM todos WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error deleting todo: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            res.code = 500;
            res.body = "{\"error\": \"Failed to delete todo\"}";
            return;
        }

        sqlite3_finalize(stmt);
        res.code = 200;
    });

    CROW_ROUTE(app, "/todo/<int>").methods("GET"_method)([&db](int id) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT id, title, completed FROM todos WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue todo;
            todo["id"] = sqlite3_column_int(stmt, 0);
            todo["title"] = (const char*)sqlite3_column_text(stmt, 1);
            todo["completed"] = sqlite3_column_int(stmt, 2);
            sqlite3_finalize(stmt);
            return crow::response{crow::json::wvalue(todo)};
        } else {
            sqlite3_finalize(stmt);
            return crow::response{404, "{\"error\": \"Todo not found\"}"};
        }
    });


    // Change port to 8080
    app.port(8080).run();

    sqlite3_close(db);
    return 0;
}
