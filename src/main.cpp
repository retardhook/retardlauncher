#include <iostream>
#include <mojang/api/api.h>
#include <mojang/auth/offline.h>
#include <mojang/game/launch.h>

int main() {
  mojang::api::Downloader downloader;
  mojang::game::Launcher launcher;
  mojang::auth::Offline offline;

  auto data = downloader.get("1.21.2", true);

  std::cout << "Client version: " << data.version << std::endl;
  std::cout << "Java version: " << data.mlv << std::endl;
  std::cout << "Client type: " << data.type << std::endl;
  std::cout << "Jar url: " << data.url << std::endl;
  std::cout << "Client json: " << data.jsonurl << std::endl;
  std::cout << "Jar path: " << data.path << std::endl;
  std::cout << "Version release date: " << data.release_time << std::endl;

  auto user = offline.Login("zerootoad");
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
