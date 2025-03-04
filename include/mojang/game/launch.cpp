#include "launch.h"
#include "mojang/auth/offline.h"
#include <cstdlib>
#include <filesystem>
#include <future>
#include <iostream>
#include <mojang/api/api.h>
#include <mojang/auth/offline.h>
#include <string>
#include <utils/utils.h>

using namespace mojang::game;

bool Launcher::launch(const mojang::auth::UserInfo &userInfo,
                      const mojang::api::JarInfo &jarInfo,
                      const std::string &javaPath) {
  std::vector<std::future<bool>> futures;
  if (!jarInfo.failed.empty()) {
    for (const auto &fail : jarInfo.failed) {
      auto [url, path] = fail;
      if (path.find("assets") == std::string::npos) {
        futures.push_back(std::async(std::launch::async, [url, path]() {
          return utils::download(url, path,
                                 path.substr(path.find_last_of('/') + 1));
        }));
      } else {
        futures.push_back(std::async(std::launch::async, [url, path]() {
          return utils::download(url, path,
                                 url.substr(path.find_last_of('/') + 1));
        }));
      }
    }
  }

  for (auto &future : futures) {
    if (!future.get()) {
      std::cout << "Missing required files, try launching again.." << std::endl;
      return false;
    }
  }

  auto classSep = utils::getOS() == "windows" ? ";" : ":";
  std::ostringstream classpathStream;
  for (const auto &lib : jarInfo.libraries) {
    if (utils::checkRules(lib.rules)) {
      classpathStream
          << std::filesystem::path(lib.path).make_preferred().string()
          << classSep;
    }
  }
  classpathStream
      << std::filesystem::path(jarInfo.path).make_preferred().string();
  std::string classpath = classpathStream.str();

  std::string command =
      javaPath + " -Xss1M -cp " + classpath + " " + jarInfo.mainclass;

  int count = 0;
  for (const auto &arg : jarInfo.arguments) {
    if (utils::checkRules(arg.rules)) {
      for (const auto &value : arg.value.values) {
        if (count < 22) {
          command += " " + value;
          count++;
        }
      }
    }
  }

  auto replacePlaceholder = [](std::string &str, const std::string &placeholder,
                               const std::string &value) {
    size_t pos = str.find(placeholder);
    if (pos != std::string::npos) {
      str.replace(pos, placeholder.length(), value);
    }
  };

  replacePlaceholder(command, "${auth_player_name}", userInfo.username);
  replacePlaceholder(command, "${version_name}", jarInfo.version);
  replacePlaceholder(
      command, "${game_directory}",
      std::filesystem::path(utils::getPaths().root).make_preferred().string());
  replacePlaceholder(command, "${assets_root}",
                     std::filesystem::path(utils::getPaths().assets)
                         .make_preferred()
                         .string());
  replacePlaceholder(command, "${assets_index_name}", jarInfo.assetindexid);
  replacePlaceholder(command, "${auth_uuid}", userInfo.uuid);
  replacePlaceholder(command, "${auth_access_token}", userInfo.access_token);
  replacePlaceholder(command, "${clientid}", "");
  replacePlaceholder(command, "${auth_xuid}", "");
  replacePlaceholder(command, "${user_type}", "msa");
  replacePlaceholder(command, "${version_type}", jarInfo.type);

  std::cout << "Launching Minecraft with command: " << command << std::endl;
  // int result = system(command.c_str());

  // if (result != 0) {
  //   return false;
  // }

  return true;
}
