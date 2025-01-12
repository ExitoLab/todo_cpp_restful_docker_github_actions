#include <crow.h>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <iostream>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;

int main() {
    mongocxx::instance instance{}; // Required for initialization
    mongocxx::uri uri("mongodb://mongo:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["todo_db"];

    crow::SimpleApp app;

    // GET /api/todos
    CROW_ROUTE(app, "/api/todos").methods(crow::HTTPMethod::GET)
    ([&db](const crow::request& req) {
        try {
            auto cursor = db["todos"].find({});
            std::vector<crow::json::wvalue> todos;

            for (auto&& doc : cursor) {
                todos.push_back(
                    crow::json::load(bsoncxx::to_json(doc))
                );
            }
            return crow::response{crow::json::wvalue(todos)};
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Database error: ") + e.what());
        }
    });

    // POST /api/todos
    CROW_ROUTE(app, "/api/todos").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        try {
            auto json = crow::json::load(req.body);
            
            document doc{};
            doc << "title" << json["title"].s()
                << "completed" << json["completed"].b();

            auto result = db["todos"].insert_one(doc.view());
            
            if (result) {
                return crow::response(201, "Todo created successfully");
            }
            return crow::response(500, "Failed to create todo");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid request: ") + e.what());
        }
    });

    // PUT /api/todos/:id
    CROW_ROUTE(app, "/api/todos/<string>").methods(crow::HTTPMethod::PUT)
    ([&db](const crow::request& req, std::string id) {
        try {
            auto json = crow::json::load(req.body);
            
            document filter{};
            filter << "_id" << bsoncxx::oid(id);

            document update{};
            update << "$set" << open_document
                   << "title" << json["title"].s()
                   << "completed" << json["completed"].b()
                   << close_document;

            auto result = db["todos"].update_one(filter.view(), update.view());
            
            if (result) {
                return crow::response(200, "Todo updated successfully");
            }
            return crow::response(404, "Todo not found");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid request: ") + e.what());
        }
    });

    // DELETE /api/todos/:id
    CROW_ROUTE(app, "/api/todos/<string>").methods(crow::HTTPMethod::DELETE)
    ([&db](const crow::request&, std::string id) {
        try {
            document filter{};
            filter << "_id" << bsoncxx::oid(id);

            auto result = db["todos"].delete_one(filter.view());
            
            if (result) {
                return crow::response(200, "Todo deleted successfully");
            }
            return crow::response(404, "Todo not found");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Invalid request: ") + e.what());
        }
    });

    app.port(8080).multithreaded().run();
    return 0;
}