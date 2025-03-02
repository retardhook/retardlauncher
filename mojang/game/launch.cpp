#include "launch.h"
#include "mojang/auth/offline.h"
#include <cstdlib>
#include <iostream>
#include <mojang/api/api.h>
#include <mojang/auth/offline.h>
#include <string>
#include <utils/utils.h>

using namespace mojang::game;

bool Launcher::launch(const mojang::auth::UserInfo &userInfo,
                      const mojang::api::JarInfo &jarInfo,
                      const std::string &javaPath) {
  std::string separator = utils::getOS() == "windows" ? ";" : ":";
  std::ostringstream classpathStream;
  for (const auto &lib : jarInfo.libraries) {
    if (utils::checkRules(lib.rules)) {
      classpathStream << lib.path << separator;
    }
  }
  classpathStream << jarInfo.path;
  std::string classpath = classpathStream.str();

  std::string command =
      javaPath + " -Xss1M -cp " + classpath + " " + jarInfo.mainclass;

  for (const auto &arg : jarInfo.arguments) {
    if (utils::checkRules(arg.rules)) {
      for (const auto &value : arg.value.values) {
        command += " " + value;
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
  replacePlaceholder(command, "${game_directory}",
                     std::filesystem::current_path().string() + jarInfo.path);
  replacePlaceholder(command, "${auth_uuid}", userInfo.uuid);
  replacePlaceholder(command, "${auth_access_token}", userInfo.access_token);
  replacePlaceholder(command, "${clientid}", "");
  replacePlaceholder(command, "${auth_xuid}", "");
  replacePlaceholder(command, "${user_type}", "msa");
  replacePlaceholder(command, "${version_type}", jarInfo.type);

  std::cout << "Launching Minecraft with command: " << command << std::endl;
  int result = system(command.c_str());

  if (result != 0) {
    return false;
  }

  return true;
}
