#define _CRT_SECURE_NO_WARNINGS
#include "launch.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utils/utils.h>

using namespace mojang::game;

bool Launcher::launch(const mojang::auth::UserInfo& userInfo,
    const mojang::api::JarInfo& jarInfo,
    const std::string& javaPath) {
    std::string separator = utils::getOS() == "windows" ? ";" : ":";
    std::ostringstream classpathStream;
    for (const auto& lib : jarInfo.libraries) {
        if (utils::checkRules(lib.rules)) {
            classpathStream << lib.path << separator;
        }
    }
    classpathStream << jarInfo.path;
    std::string classpath = classpathStream.str();

    std::cout << jarInfo.path.c_str() << std::endl;

    std::string command =
        javaPath + " -Xss1M -cp " + classpath + " " + jarInfo.mainclass;

    int count = 0;
    for (const auto& arg : jarInfo.arguments) {
        if (utils::checkRules(arg.rules)) {
            for (const auto& value : arg.value.values) {
                if (count < 22) {
                    command += " " + value;
                    count++;
                }
            }
        }
    }

    auto replacePlaceholder = [](std::string& str, const std::string& placeholder,
        const std::string& value) {
            size_t pos = str.find(placeholder);
            if (pos != std::string::npos) {
                str.replace(pos, placeholder.length(), value);
            }
        };

    replacePlaceholder(command, "${auth_player_name}", userInfo.username);
    replacePlaceholder(command, "${version_name}", jarInfo.version);
    replacePlaceholder(command, "${game_directory}",
        get_game_dir() + "/versions/" +
        jarInfo.version);
    replacePlaceholder(command, "${assets_root}",
        get_game_dir() + "/versions/" +
        jarInfo.version + "/assets");
    replacePlaceholder(command, "${assets_index_name}", jarInfo.assetindexid);
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

std::string Launcher::get_game_dir() {
    std::string gameDir;
    char* temp = nullptr;

    if (utils::getOS() == "windows") {
        size_t len;
        if (_dupenv_s(&temp, &len, "APPDATA") == 0 && temp != nullptr) {
            gameDir = temp;
            gameDir += "\\.retardlauncher";

            std::cout << gameDir << std::endl;
            free(temp);
        }
    }
    else if (utils::getOS() == "linux") {
        gameDir = std::getenv("HOME");
        if (!gameDir.empty()) {
            gameDir += "/.retardlauncher";
        }
    }
    else if (utils::getOS() == "osx") {
        gameDir = std::getenv("HOME");
        if (!gameDir.empty()) {
            gameDir += "/Library/Application Support/retardlauncher";
        }
    }
    return gameDir;
}
