#pragma once
#include <string>
#include <vector>

namespace mojang::api {
	// https://launchercontent.mojang.com/v2/javaPatchNotes/id.json
	struct NewsInfo {
		std::string title;
		std::string short_desc;
		std::string date;
		std::string id;
		std::string image_url;
		std::string body;
	};
	class News {
	public:
		std::vector<NewsInfo> get();
	};;
}