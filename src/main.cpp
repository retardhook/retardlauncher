#include <httplib/httplib.h>
#include <mojang/api/download.h>
#include <mojang/api/news.h>
#include <mojang/auth/offline.h>
#include <mojang/game/launch.h>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Helper function to set CORS headers
void set_cors_headers(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

int main() {
    httplib::Server svr;

    svr.Options("/download", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        res.status = 200;
        res.set_content("", "text/plain");
        });

    svr.Get("/download", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        nlohmann::json request_json;
        try {
            request_json = nlohmann::json::parse(req.body);
        }
        catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid JSON\"}", "application/json");
            return;
        }

        std::string version = request_json["version"];
        mojang::api::Downloader downloader;
        auto data = downloader.get(version, true);

        nlohmann::json response;
        response["version"] = data.version;
        response["java_version"] = data.mlv;
        response["client_type"] = data.type;
        response["jar_url"] = data.url;
        response["client_json"] = data.jsonurl;
        response["jar_path"] = data.path;
        response["release_time"] = data.release_time;

        res.set_content(response.dump(), "application/json");
        });

    svr.Get("/news", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        mojang::api::News news;
        std::vector<mojang::api::NewsInfo> entries = news.get();

        nlohmann::json response;
        for (size_t i = 0; i < entries.size(); i++) {
            response[i]["title"] = entries[i].title;
            response[i]["short_desc"] = entries[i].short_desc;
            response[i]["date"] = entries[i].date;
            response[i]["body"] = entries[i].body;
            response[i]["image_url"] = entries[i].image_url;
        }

        res.set_content(response.dump(), "application/json");
        });

    svr.Post("/launch", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        nlohmann::json request_json;
        try {
            request_json = nlohmann::json::parse(req.body);
        }
        catch (const std::exception& e) {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid JSON\"}", "application/json");
            return;
        }

        std::string version = request_json["version"];
        std::string username = request_json["username"];

        std::cout << "launch endpoint called with version " << version << " and username " << username << std::endl;

        mojang::auth::Offline offline;
        mojang::game::Launcher launcher;

        auto user = offline.Login(username);
        auto data = mojang::api::Downloader().get(version, true);

        launcher.launch(user, data, "java");

        nlohmann::json response;
        response["status"] = "success";
        response["message"] = "Launched Minecraft for " + username;

        res.set_content(response.dump(), "application/json");
        });

    svr.Get("/versions", [](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        std::cout << "versions fetch was called" << std::endl;

        cpr::Response r = cpr::Get(cpr::Url{
            "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json" });

        if (r.status_code == 200 && !r.text.empty()) {
            nlohmann::json j = nlohmann::json::parse(r.text);

            nlohmann::json response;
            if (j.contains("versions")) {
                size_t index = 0;
                for (auto& version_info : j["versions"]) {
                    if (version_info.contains("id")) {
                        response[index++] = version_info["id"];
                    }
                }
            }

            res.set_content(response.dump(), "application/json");
        }
        else {
            res.status = 500;
            res.set_content("{\"error\": \"Failed to fetch versions\"}", "application/json");
        }
        });

    std::cout << "Server running on http://localhost:45932" << std::endl;
    svr.listen("0.0.0.0", 45932);

    return 0;
}
