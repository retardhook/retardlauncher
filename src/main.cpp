#include <iostream>
#include <mojang/api/download.h>
#include <mojang/api/news.h>
#include <mojang/auth/offline.h>
#include <mojang/auth/online.h>
#include <mojang/game/launch.h>

int main() {
	mojang::api::Downloader downloader;
	mojang::game::Launcher launcher;
	mojang::auth::Offline offline;
	mojang::auth::Online online;
	mojang::api::News news;

	std::vector<mojang::api::NewsInfo> entries = news.get();

	for (const auto& entry : entries) {
		std::cout << "Title: " << entry.title << std::endl;
		std::cout << "Short Description: " << entry.short_desc << std::endl;
		std::cout << "Date: " << entry.date << std::endl;
		std::cout << "Body: " << entry.body << std::endl;
		std::cout << "Image URL: " << entry.image_url << std::endl;
		std::cout << "----------------------------------------" << std::endl;
	}

	auto data = downloader.get("1.21.1", true);

	std::cout << "Client version: " << data.version << std::endl;
	std::cout << "Java version: " << data.mlv << std::endl;
	std::cout << "Client type: " << data.type << std::endl;
	std::cout << "Jar url: " << data.url << std::endl;
	std::cout << "Client json: " << data.jsonurl << std::endl;
	std::cout << "Jar path: " << data.path << std::endl;
	std::cout << "Version release date: " << data.release_time << std::endl;

	auto user = offline.Login("nyrilol");
	launcher.launch(user, data, "java");

	// for (auto i : data.libraries) {
	//   std::cout << "Library name: " << i.name << std::endl;
	//   std::cout << "Library url: " << i.url << std::endl;
	//   std::cout << "Library path: " << i.path << std::endl;
	// }

	// std::cout << "Arguments: ";
	// for (auto i : data.arguments) {
	//   for (auto j : i.value.values) {
	//     std::cout << j << " ";
	//   }
	// }
}
