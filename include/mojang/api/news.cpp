#include "news.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <future>
#include <vector>
#include <stdexcept>

using namespace mojang::api;
using json = nlohmann::json;

std::vector<NewsInfo> News::get() {
    cpr::Response response = cpr::Get(cpr::Url{ "https://launchercontent.mojang.com/v2/javaPatchNotes.json" });

    if (response.status_code != 200) {
        throw std::runtime_error("Failed to fetch news data");
    }

    json news_json = json::parse(response.text);
    std::vector<std::future<NewsInfo>> futures;

    for (const auto& entry : news_json["entries"]) {
        futures.push_back(std::async(std::launch::async, [entry]() -> NewsInfo {
            std::string content_url = "https://launchercontent.mojang.com/v2/" + entry["contentPath"].get<std::string>();
            cpr::Response content_response = cpr::Get(cpr::Url{ content_url });

            if (content_response.status_code != 200) {
                throw std::runtime_error("Failed to fetch content data for entry: " + entry["id"].get<std::string>());
            }

            json content_json = json::parse(content_response.text);
            NewsInfo news_info;
            news_info.title = entry["title"];
            news_info.short_desc = entry["shortText"];
            news_info.date = entry["date"];
            news_info.id = entry["id"];
            news_info.image_url = "https://launchercontent.mojang.com/v2" + entry["image"]["url"].get<std::string>();
            news_info.body = content_json["body"];

            return news_info;
            }));
    }

    std::vector<NewsInfo> news_entries;
    for (auto& future : futures) {
        news_entries.push_back(future.get());
    }

    return news_entries;
}
